/**
 * Copyright (c) 2010 William Light <wrl@illest.net>
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
#include <stdlib.h>
#include <stdint.h>
#include <lo/lo.h>

#include <monome.h>
#include "internal.h"

#include "osc.h"

#define SELF_FROM(what_okay) monome_osc_t *self = (monome_osc_t *) what_okay;
#define LO_SEND_MSG(type, ...) lo_send_from(self->outgoing, self->server, LO_TT_IMMEDIATE, self->type##_str, __VA_ARGS__)

static int proto_osc_close(monome_t *monome);
static void proto_osc_free(monome_t *monome);

/**
 * private
 */

static void proto_osc_lo_error(int num, const char *error_msg, const char *path) {
	fprintf(stderr, "libmonome: liblo server error %d in %s:\n\t%s\n", num, path, error_msg);
	fflush(stderr);
}

static int proto_osc_press_handler(const char *path, const char *types, lo_arg **argv, int argc, lo_message data, void *user_data) {
	SELF_FROM(user_data);
	monome_event_t *e = self->e_ptr;

	if( !e )
		return 1;

	e->x          = argv[0]->i;
	e->y          = argv[1]->i;
	e->event_type = argv[2]->i & 1;

	self->have_event = 1;
	return 0;
}

/**
 * public
 */

static int proto_osc_clear(monome_t *monome, monome_clear_status_t status) {
	SELF_FROM(monome);
	return LO_SEND_MSG(clear, "i", status);
}

static int proto_osc_intensity(monome_t *monome, uint brightness) {
	SELF_FROM(monome);
	return LO_SEND_MSG(intensity, "i", brightness);
}

static int proto_osc_mode(monome_t *monome, monome_mode_t mode) {
	SELF_FROM(monome);

	/* sys message?  why? */
	return lo_send_from(self->outgoing, self->server, LO_TT_IMMEDIATE, "/sys/mode", "i", mode);
}

static int proto_osc_led_on(monome_t *monome, uint x, uint y) {
	SELF_FROM(monome);
	return LO_SEND_MSG(led, "iii", x, y, 1);
}

static int proto_osc_led_off(monome_t *monome, uint x, uint y) {
	SELF_FROM(monome);
	return LO_SEND_MSG(led, "iii", x, y, 0);
}

static int proto_osc_led_col(monome_t *monome, uint col, size_t count, const uint8_t *data) {
	SELF_FROM(monome);

	if( count == 1 )
		return LO_SEND_MSG(led_col, "ii", col, data[0]);

	return LO_SEND_MSG(led_col, "iii", col, data[0], data[1]);
}

static int proto_osc_led_row(monome_t *monome, uint row, size_t count, const uint8_t *data) {
	SELF_FROM(monome);

	if( count == 1 )
		return LO_SEND_MSG(led_row, "ii", row, data[0]);

	return LO_SEND_MSG(led_row, "iii", row, data[0], data[1]);
}

static int proto_osc_led_frame(monome_t *monome, uint quadrant, const uint8_t *f) {
	SELF_FROM(monome);

	/* there has to be a cleaner way to do this */
	return LO_SEND_MSG(frame, "iiiiiiiii", f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], quadrant);
}

static int proto_osc_next_event(monome_t *monome, monome_event_t *e) {
	SELF_FROM(monome);

	self->e_ptr = e;
	self->have_event = 0;

	lo_server_recv_noblock(self->server, 0);

	return self->have_event;
}

static int proto_osc_open(monome_t *monome, const char *dev,
						  const char *serial, const monome_devmap_t *m,
						  va_list args) {
	SELF_FROM(monome);
	char *port, *buf;

	port = va_arg(args, char *);

	if( !(self->server = lo_server_new(port, proto_osc_lo_error)) )
		return 1;

	self->prefix   = lo_url_get_path(dev);
	self->outgoing = lo_address_new_from_url(dev);

	if( (monome->fd = lo_server_get_socket_fd(self->server)) < 0 ) {
		proto_osc_close(monome);
		proto_osc_free(monome);
		return 1;
	}

	asprintf(&buf, "%s/press", self->prefix);
	lo_server_add_method(self->server, buf, "iii", proto_osc_press_handler, self);
	free(buf);

#define cache_osc_path(base) asprintf(&self->base##_str, "%s/" #base, self->prefix)
	cache_osc_path(clear);
	cache_osc_path(intensity);
	cache_osc_path(mode);
	cache_osc_path(led);
	cache_osc_path(led_row);
	cache_osc_path(led_col);
	cache_osc_path(frame);
#undef cache_osc_path

	return 0;
}

static int proto_osc_close(monome_t *monome) {
	return 0;
}

static void proto_osc_free(monome_t *monome) {
	SELF_FROM(monome);

#define clear_osc_path(base) free(self->base##_str);
	clear_osc_path(clear);
	clear_osc_path(intensity);
	clear_osc_path(mode);
	clear_osc_path(led);
	clear_osc_path(led_row);
	clear_osc_path(led_col);
	clear_osc_path(frame);
#undef clear_osc_path

	free(self->prefix);
	lo_server_free(self->server);
	lo_address_free(self->outgoing);

	self->prefix   = NULL;
	self->server   = NULL;
	self->outgoing = NULL;

	free(self);
}

monome_t *monome_protocol_new() {
	monome_osc_t *self = calloc(1, sizeof(monome_osc_t));
	monome_t *monome = (monome_t *) self;
	
	if( !monome )
		return NULL;
	
	monome->open       = proto_osc_open;
	monome->close      = proto_osc_close;
	monome->free       = proto_osc_free;

	monome->next_event = proto_osc_next_event;

	monome->clear      = proto_osc_clear;
	monome->intensity  = proto_osc_intensity;
	monome->mode       = proto_osc_mode;
	
	monome->led_on     = proto_osc_led_on;
	monome->led_off    = proto_osc_led_off;
	monome->led_col    = proto_osc_led_col;
	monome->led_row    = proto_osc_led_row;
	monome->led_frame  = proto_osc_led_frame;

	return monome;
}
