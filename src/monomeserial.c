/*
 * Copyright (c) 2007-2010, William Light <will@visinin.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>

#include <getopt.h>
#include <lo/lo.h>

#include <monome.h>

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
	lo_address *outgoing;
	lo_server *server;

	char *lo_prefix;
} ms_state;

ms_state state;

static void lo_error(int num, const char *error_msg, const char *path) {
	printf("monomeserial: lo server error %d in %s: %s\n",
		   num, path, error_msg);
	fflush(stdout);
}

static int osc_clear_handler(const char *path, const char *types,
							 lo_arg **argv, int argc,
							 lo_message data, void *user_data) {
	monome_t *monome = user_data;
	int mode = (argc) ? argv[0]->i : 0;

	return monome_clear(monome, mode);
}

static int osc_intensity_handler(const char *path, const char *types,
								 lo_arg **argv, int argc,
								 lo_message data, void *user_data) {
	monome_t *monome = user_data;
	int intensity = (argc) ? argv[0]->i : 0xF;

	return monome_intensity(monome, intensity);
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

	if( argv[2]->i )
		return monome_led_on(monome, argv[0]->i, argv[1]->i);
	else
		return monome_led_off(monome, argv[0]->i, argv[1]->i);
}

static int osc_led_col_row_handler(const char *path, const char *types,
								   lo_arg **argv, int argc,
								   lo_message data, void *user_data) {
	monome_t *monome = user_data;
	uint8_t buf[2] = {argv[1]->i};

	if( argc == 3 )
		buf[1] = argv[2]->i;

	if( strstr(path, "led_col") )
		return monome_led_col(monome, argv[0]->i, argc - 1, buf);
	else
		return monome_led_row(monome, argv[0]->i, argc - 1, buf);
}

static int osc_frame_handler(const char *path, const char *types,
							 lo_arg **argv, int argc,
							 lo_message data, void *user_data) {
	monome_t *monome = user_data;
	uint8_t buf[8];
	uint i;

	for( i = 0; i < 8; i++ )
		buf[i] = argv[i]->i;

	switch( argc ) {
	case 8:
		return monome_led_frame(monome, 0, buf);
		break;

	case 9:
		return monome_led_frame(monome, argv[8]->i, buf);
		break;

	case 10:
		/**
		 * okay, this isn't implemented yet.
		 * passing 10 arguments to /frame means you want to offset
		 * it by argv[8] and argv[9].
		 * 
		 * thing is, there's no clean mapping to the serial protocol,
		 * so this is going to have to wait.
		 */
		break;
	}

	return -1;
}

static void register_osc_methods(char *prefix, monome_t *monome) {
	lo_server_thread srv = state.server;
	char *cmd_buf;

	asprintf(&cmd_buf, "/%s/clear", prefix);
	lo_server_add_method(srv, cmd_buf, "", osc_clear_handler, monome);
	lo_server_add_method(srv, cmd_buf, "i", osc_clear_handler, monome);
	free(cmd_buf);

	asprintf(&cmd_buf, "/%s/intensity", prefix);
	lo_server_add_method(srv, cmd_buf, "", osc_intensity_handler, monome);
	lo_server_add_method(srv, cmd_buf, "i", osc_intensity_handler, monome);
	free(cmd_buf);

	asprintf(&cmd_buf, "/%s/led", prefix);
	lo_server_add_method(srv, cmd_buf, "iii", osc_led_handler, monome);
	free(cmd_buf);

	asprintf(&cmd_buf, "/%s/led_row", prefix);
	lo_server_add_method(srv, cmd_buf, "ii", osc_led_col_row_handler, monome);
	lo_server_add_method(srv, cmd_buf, "iii", osc_led_col_row_handler, monome);
	free(cmd_buf);

	asprintf(&cmd_buf, "/%s/led_col", prefix);
	lo_server_add_method(srv, cmd_buf, "ii", osc_led_col_row_handler, monome);
	lo_server_add_method(srv, cmd_buf, "iii", osc_led_col_row_handler, monome);
	free(cmd_buf);

	asprintf(&cmd_buf, "/%s/frame", prefix);
	lo_server_add_method(srv, cmd_buf, "iiiiiiii", osc_frame_handler, monome);
	lo_server_add_method(srv, cmd_buf, "iiiiiiiii", osc_frame_handler, monome);
	lo_server_add_method(srv, cmd_buf, "iiiiiiiiii",
						 osc_frame_handler, monome);
	free(cmd_buf);
}

static void unregister_osc_methods(char *prefix) {
	lo_server_thread srv = state.server;
	char *cmd_buf;

	asprintf(&cmd_buf, "/%s/clear", prefix);
	lo_server_del_method(srv, cmd_buf, "");
	lo_server_del_method(srv, cmd_buf, "i");
	free(cmd_buf);

	asprintf(&cmd_buf, "/%s/intensity", prefix);
	lo_server_del_method(srv, cmd_buf, "");
	lo_server_del_method(srv, cmd_buf, "i");
	free(cmd_buf);

	asprintf(&cmd_buf, "/%s/led", prefix);
	lo_server_del_method(srv, cmd_buf, "iii");
	free(cmd_buf);

	asprintf(&cmd_buf, "/%s/led_row", prefix);
	lo_server_del_method(srv, cmd_buf, "ii");
	lo_server_del_method(srv, cmd_buf, "iii");
	free(cmd_buf);

	asprintf(&cmd_buf, "/%s/led_col", prefix);
	lo_server_del_method(srv, cmd_buf, "ii");
	lo_server_del_method(srv, cmd_buf, "iii");
	free(cmd_buf);

	asprintf(&cmd_buf, "/%s/frame", prefix);
	lo_server_del_method(srv, cmd_buf, "iiiiiiii");
	lo_server_del_method(srv, cmd_buf, "iiiiiiiii");
	lo_server_del_method(srv, cmd_buf, "iiiiiiiiii");
	free(cmd_buf);
}

static int sys_mode_handler(const char *path, const char *types,
							 lo_arg **argv, int argc,
							 lo_message data, void *user_data) {
	monome_t *monome = user_data;

	return monome_mode(monome, argv[0]->i);
}

static void register_sys_methods(monome_t *monome) {
	lo_server_thread srv = state.server;

	lo_server_add_method(srv, "/sys/mode", "i", sys_mode_handler, monome);

	return;
}

static void monome_handle_press(const monome_event_t *e, void *data) {
	char *cmd;
	char *prefix = data;

	asprintf(&cmd, "/%s/press", prefix);
	lo_send_from(state.outgoing, state.server, LO_TT_IMMEDIATE, cmd, "iii",
				 e->x, e->y, e->event_type);
	free(cmd);
}

static void usage(const char *app) {
	printf("usage: %s [options...] [prefix]\n"
		   "\n"
		   "  -h, --help			display this information\n"
		   "\n"
		   "  -d, --device <device>		the monome serial device\n"
		   "  -p, --protocol <protocol>	which protocol to use"
		       "(\"40h\" or \"series\")\n"
		   "\n"
		   "  -s, --server-port <port>	what port to listen on\n"
		   "  -a, --application-port <port>	what port to talk to\n"
		   "  -o, --application-host <host> the host your application is on\n"
		   "\n"
		   "  -r, --orientation <direction>	one of "
		       "\"left\", \"right\", \"bottom\", or \"top\"\n"
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

static int main_loop() {
	struct pollfd fds[2];

	fds[0].fd = monome_get_fd(state.monome);
	fds[1].fd = lo_server_get_socket_fd(state.server);

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
			lo_server_recv_noblock(state.server, 0);

	} while( 1 );
}

int main(int argc, char *argv[]) {
	char c, *device, *sport, *aport, *ahost, *proto;
	monome_cable_t orientation = MONOME_CABLE_LEFT;
	int i;

	struct option arguments[] = {
		{"help",             no_argument,       0, 'h'},

		{"device",           required_argument, 0, 'd'},
		{"protocol",         required_argument, 0, 'p'},

		{"server-port",      required_argument, 0, 's'},
		{"application-port", required_argument, 0, 'a'},
		{"application-host", required_argument, 0, 'o'},

		{"orientation",      required_argument, 0, 'r'}
	};

	device = DEFAULT_MONOME_DEVICE;
	proto  = DEFAULT_MONOME_PROTOCOL;
	sport  = DEFAULT_OSC_SERVER_PORT;
	aport  = DEFAULT_OSC_APP_PORT;
	ahost  = DEFAULT_OSC_APP_HOST;

	while( (c = getopt_long(argc, argv, "hd:p:s:a:o:r:",
							arguments, &i)) > 0 ) {
		switch( c ) {
		case 'h':
			usage(argv[0]);
			return 1;

		case 'd':
			device = optarg;
			break;

		case 'p':
			proto = optarg;
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
			case 'l': orientation = MONOME_CABLE_LEFT;   break;
			case 'b': orientation = MONOME_CABLE_BOTTOM; break;
			case 'r': orientation = MONOME_CABLE_RIGHT;  break;
			case 't': orientation = MONOME_CABLE_TOP;    break;
			}
			break;
		}
	}

	if( optind == argc )
		state.lo_prefix = strdup(DEFAULT_OSC_PREFIX);
	else
		state.lo_prefix = strdup(argv[optind]);

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

	register_sys_methods(state.monome);
	register_osc_methods(state.lo_prefix, state.monome);

	monome_set_orientation(state.monome, orientation);
	monome_clear(state.monome, MONOME_CLEAR_OFF);
	monome_mode(state.monome, MONOME_MODE_NORMAL);

	printf("monomeserial version %s, yay!\n\n", VERSION);
	printf("initialized device %s at %s, which is %dx%d\n",
		   monome_get_serial(state.monome), monome_get_devpath(state.monome),
		   monome_get_rows(state.monome), monome_get_cols(state.monome));
	printf("running with prefix /%s\n\n", state.lo_prefix);

	/* main_loop() returns 1 if the monome was disconnected */
	if( !main_loop() )
		monome_close(state.monome);

	unregister_osc_methods(state.lo_prefix);
	free(state.lo_prefix);

	return EXIT_SUCCESS;
}
