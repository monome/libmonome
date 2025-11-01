/**
 * Copyright (c) 2010 William Light <wrl@illest.net>
 * Copyright (c) 2013 Nedko Arnaudov <nedko@arnaudov.name>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>

#include <getopt.h>
#include <lo/lo.h>

#include <monome.h>
#include "platform.h"

#define DEFAULT_MONOME_DEVICE   "/dev/ttyUSB0"
#define DEFAULT_MONOME_PROTOCOL "series"

#define DEFAULT_OSC_PREFIX      "monome"
#define DEFAULT_OSC_SERVER_PORT "8080"
#define DEFAULT_OSC_APP_PORT    "8000"
#define DEFAULT_OSC_APP_HOST    "127.0.0.1"

#ifdef DEBUG
#define DPRINTF(...) fprintf(stderr, __VA_ARGS__)
#else
#define DPRINTF(...) ((void) 0)
#endif

typedef struct {
	monome_t *monome;
	lo_address outgoing;
	lo_server server;

	char *lo_prefix;
} ms_state;

ms_state state;

static void lo_error(int num, const char *error_msg, const char *path) {
	printf("monomeserial: lo server error %d in %s: %s\n",
		   num, path, error_msg);
	fflush(stdout);
}

static int osc_led_handler(const char *path, const char *types,
						   lo_arg **argv, int argc,
						   lo_message data, void *user_data) {
	monome_t *monome = user_data;

	if( (argc != 3 || strcmp("iii", types)) ||
		(argv[0]->i > 15 || argv[0]->i < 0) ||
		(argv[1]->i > 15 || argv[1]->i < 0) ||
		(argv[2]->i > 1  || argv[2]->i < 0) )
		return -1;

	return monome_led_set(monome, argv[0]->i, argv[1]->i, argv[2]->i);
}

static int osc_led_all_handler(const char *path, const char *types,
							 lo_arg **argv, int argc,
							 lo_message data, void *user_data) {
	monome_t *monome = user_data;
	int mode = (argc) ? argv[0]->i : 0;

	return monome_led_all(monome, mode);
}

static int osc_led_col_row_handler(const char *path, const char *types,
								   lo_arg **argv, int argc,
								   lo_message data, void *user_data) {
	monome_t *monome = user_data;
	uint8_t buf[2] = {argv[1]->i};

	if( argc == 3 )
		buf[1] = argv[2]->i;

	if( strstr(path, "led_col") )
		return monome_led_col(monome, argv[0]->i, 0, argc - 1, buf);
	else
		return monome_led_row(monome, 0, argv[0]->i, argc - 1, buf);
}

static int osc_led_map_handler(const char *path, const char *types,
                               lo_arg **argv, int argc,
                               lo_message data, void *user_data) {
	monome_t *monome = user_data;
	uint8_t buf[8];
	uint i;

	for( i = 0; i < 8; i++ )
		buf[i] = argv[i + (argc - 8)]->i;

	switch( argc ) {
	case 8:
		return monome_led_map(monome, 0, 0, buf);

	case 10:
		return monome_led_map(monome, argv[0]->i, argv[1]->i, buf);
	}

	return -1;
}

static int osc_intensity_handler(const char *path, const char *types,
								 lo_arg **argv, int argc,
								 lo_message data, void *user_data) {
	monome_t *monome = user_data;
	int intensity = (argc) ? argv[0]->i : 0xF;

	return monome_led_intensity(monome, intensity);
}

#define ASPRINTF_OR_BAIL(...) do { \
	if (asprintf(__VA_ARGS__) < 0) \
		return;                    \
	} while (0);

static void register_osc_methods(char *prefix, monome_t *monome) {
	lo_server srv = (lo_server)state.server;
	char *cmd_buf;

	ASPRINTF_OR_BAIL(&cmd_buf, "/%s/led", prefix);
	lo_server_add_method(srv, cmd_buf, "iii", osc_led_handler, monome);
	m_free(cmd_buf);

	ASPRINTF_OR_BAIL(&cmd_buf, "/%s/clear", prefix);
	lo_server_add_method(srv, cmd_buf, "", osc_led_all_handler, monome);
	lo_server_add_method(srv, cmd_buf, "i", osc_led_all_handler, monome);
	m_free(cmd_buf);

	ASPRINTF_OR_BAIL(&cmd_buf, "/%s/frame", prefix);
	lo_server_add_method(srv, cmd_buf, "iiiiiiii", osc_led_map_handler, monome);
	lo_server_add_method(srv, cmd_buf, "iiiiiiiiii",
						 osc_led_map_handler, monome);
	m_free(cmd_buf);

	ASPRINTF_OR_BAIL(&cmd_buf, "/%s/led_row", prefix);
	lo_server_add_method(srv, cmd_buf, "ii", osc_led_col_row_handler, monome);
	lo_server_add_method(srv, cmd_buf, "iii", osc_led_col_row_handler, monome);
	m_free(cmd_buf);

	ASPRINTF_OR_BAIL(&cmd_buf, "/%s/led_col", prefix);
	lo_server_add_method(srv, cmd_buf, "ii", osc_led_col_row_handler, monome);
	lo_server_add_method(srv, cmd_buf, "iii", osc_led_col_row_handler, monome);
	m_free(cmd_buf);

	ASPRINTF_OR_BAIL(&cmd_buf, "/%s/intensity", prefix);
	lo_server_add_method(srv, cmd_buf, "", osc_intensity_handler, monome);
	lo_server_add_method(srv, cmd_buf, "i", osc_intensity_handler, monome);
	m_free(cmd_buf);
}

static void unregister_osc_methods(char *prefix) {
	lo_server srv = (lo_server)state.server;
	char *cmd_buf;

	ASPRINTF_OR_BAIL(&cmd_buf, "/%s/clear", prefix);
	lo_server_del_method(srv, cmd_buf, "");
	lo_server_del_method(srv, cmd_buf, "i");
	m_free(cmd_buf);

	ASPRINTF_OR_BAIL(&cmd_buf, "/%s/intensity", prefix);
	lo_server_del_method(srv, cmd_buf, "");
	lo_server_del_method(srv, cmd_buf, "i");
	m_free(cmd_buf);

	ASPRINTF_OR_BAIL(&cmd_buf, "/%s/led", prefix);
	lo_server_del_method(srv, cmd_buf, "iii");
	m_free(cmd_buf);

	ASPRINTF_OR_BAIL(&cmd_buf, "/%s/led_row", prefix);
	lo_server_del_method(srv, cmd_buf, "ii");
	lo_server_del_method(srv, cmd_buf, "iii");
	m_free(cmd_buf);

	ASPRINTF_OR_BAIL(&cmd_buf, "/%s/led_col", prefix);
	lo_server_del_method(srv, cmd_buf, "ii");
	lo_server_del_method(srv, cmd_buf, "iii");
	m_free(cmd_buf);

	ASPRINTF_OR_BAIL(&cmd_buf, "/%s/frame", prefix);
	lo_server_del_method(srv, cmd_buf, "iiiiiiii");
	lo_server_del_method(srv, cmd_buf, "iiiiiiiiii");
	m_free(cmd_buf);

}

static void monome_handle_press(const monome_event_t *e, void *data) {
	char *cmd;
	char *prefix = data;

	ASPRINTF_OR_BAIL(&cmd, "/%s/press", prefix);
	lo_send_from(state.outgoing, (lo_server)state.server, LO_TT_IMMEDIATE, cmd, "iii",
				 e->grid.x, e->grid.y, e->event_type);
	m_free(cmd);
}

#undef ASPRINTF_OR_BAIL

static void usage(const char *app) {
	printf(
		"usage: %s [options...] [prefix]\n"
		"\n"
		"  -h, --help			display this information\n"
		"\n"
		"  -d, --device <device>		the monome serial device\n"

		/*
		 * protocol cannot currently be explicitly specified.
		 * this functionality will be re-added at a later date.
		 *

		"  -p, --protocol <protocol>	which protocol to use"
			"(\"40h\" or \"series\")\n" */

		"\n"
		"  -s, --server-port <port>	what port to listen on\n"
		"  -a, --application-port <port>	what port to talk to\n"
		"  -o, --application-host <host> the host your application is on\n"
		"\n"
		"  -r, --rotation <degrees>	rotate the monome. "
			"degrees can only be one of 0, 90, 180, or 270.\n"
		"\n", app);
}

static int is_numstr(const char *s) {
	while((48 <= *s) && (*s++ <= 57)); /* 48 is ASCII '0', 57 is '9' */

	/* if the character we stopped on isn't a null,
	   we didn't make it through the string */
	if( *s )
		return 0; /* oh well :( */
	return 1;
}

/* on OSX, poll() does not work with devices (i.e. ttys). */

#ifndef HAVE_BROKEN_POLL
static int main_loop() {
	struct pollfd fds[2];

	fds[0].fd = monome_get_fd(state.monome);
	fds[1].fd = lo_server_get_socket_fd((lo_server)state.server);

	fds[0].events = fds[1].events = 
		POLLIN;

	do {
		/* block until either the monome or liblo have data */
		poll(fds, 2, -1);

		/* is the monome still connected? */
		if( fds[0].revents & (POLLHUP | POLLERR) )
			return 1;

		/* is there data available for reading from the monome? */
		if( fds[0].revents & POLLIN )
			monome_event_handle_next(state.monome);

		/* how about from OSC? */
		if( fds[1].revents & POLLIN )
			lo_server_recv_noblock((lo_server)state.server, 0);
	} while( 1 );
}
#else
static int main_loop() {
	fd_set rfds, efds;
	int maxfd, mfd, lofd;

	mfd  = monome_get_fd(state.monome);
	lofd = lo_server_get_socket_fd((lo_server)state.server);
	maxfd = ((lofd > mfd) ? lofd : mfd) + 1;

	do {
		FD_ZERO(&rfds);
		FD_SET(mfd, &rfds);
		FD_SET(lofd, &rfds);

		FD_ZERO(&efds);
		FD_SET(mfd, &efds);

		/* block until either the monome or liblo have data */
		select(maxfd, &rfds, NULL, &efds, NULL);

		/* is the monome still connected? */
		if( FD_ISSET(mfd, &efds) )
			return 1;

		/* is there data available for reading from the monome? */
		if( FD_ISSET(mfd, &rfds) )
			monome_event_handle_next(state.monome);

		/* how about from OSC? */
		if( FD_ISSET(lofd, &rfds) )
			lo_server_recv_noblock((lo_server)state.server, 0);
	} while( 1 );
}
#endif

int main(int argc, char *argv[]) {
	char *device, *sport, *aport, *ahost;
	monome_rotate_t rotate = MONOME_ROTATE_0;
	int c, i;

	struct option arguments[] = {
		{"help",             no_argument,       0, 'h'},

		{"device",           required_argument, 0, 'd'},

		{"server-port",      required_argument, 0, 's'},
		{"application-port", required_argument, 0, 'a'},
		{"application-host", required_argument, 0, 'o'},

		{"rotation",         required_argument, 0, 'r'}
	};

	device = DEFAULT_MONOME_DEVICE;
	sport  = DEFAULT_OSC_SERVER_PORT;
	aport  = DEFAULT_OSC_APP_PORT;
	ahost  = DEFAULT_OSC_APP_HOST;

	while( (c = getopt_long(argc, argv, "hd:s:a:o:r:",
							arguments, &i)) > 0 ) {
		switch( c ) {
		case 'h':
			usage(argv[0]);
			return 1;

		case 'd':
			device = optarg;
			break;

		case 's':
			if( is_numstr(optarg) )
				sport = optarg;
			else
				printf("warning: \"%s\" is not a valid server port.\n",
					   optarg);

			break;

		case 'a':
			if( is_numstr(optarg) )
				aport = optarg;
			else
				printf("warning: \"%s\" is not a valid application port.\n",
					   optarg);

			break;

		case 'o':
			ahost = optarg;
			break;

		case 'r':
			switch(*optarg) {
			case 'l': case '0': rotate = MONOME_ROTATE_0;   break;
			case 't': case '9': rotate = MONOME_ROTATE_90;  break;
			case 'r': case '1': rotate = MONOME_ROTATE_180; break;
			case 'b': case '2': rotate = MONOME_ROTATE_270; break;
			}
			break;
		}
	}

	if( optind == argc )
		state.lo_prefix = m_strdup(DEFAULT_OSC_PREFIX);
	else
		state.lo_prefix = m_strdup(argv[optind]);

	if( !(state.monome = monome_open(device)) ) {
		printf("failed to open %s\n", device);
		return EXIT_FAILURE;
	}

	if( !(state.server = lo_server_new(sport, lo_error)) )
		return EXIT_FAILURE;

	state.outgoing = lo_address_new(ahost, aport);

	monome_register_handler(state.monome, MONOME_BUTTON_DOWN,
							monome_handle_press, state.lo_prefix);
	monome_register_handler(state.monome, MONOME_BUTTON_UP,
							monome_handle_press, state.lo_prefix);

	register_osc_methods(state.lo_prefix, state.monome);

	monome_set_rotation(state.monome, rotate);
	monome_led_all(state.monome, 0);

	printf("monomeserial version %s, yay!\n\n", VERSION);
	printf("initialized device %s (%s) at %s, which is %dx%d using proto %s\n",
		   monome_get_serial(state.monome), monome_get_friendly_name(state.monome),
		   monome_get_devpath(state.monome),
		   monome_get_rows(state.monome), monome_get_cols(state.monome),
		   monome_get_proto(state.monome));
	printf("running with prefix /%s\n\n", state.lo_prefix);

	/* main_loop() returns 1 if the monome was disconnected */
	if( main_loop() )
		printf("%s disconnected, monomeserial exiting.\nsee you later!\n\n",
			   monome_get_devpath(state.monome));

	monome_close(state.monome);

	unregister_osc_methods(state.lo_prefix);
	m_free(state.lo_prefix);

	lo_address_free(state.outgoing);
	lo_server_free(state.server);

	return EXIT_SUCCESS;
}
