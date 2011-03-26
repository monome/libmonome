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
#include "platform.h"
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

	e->grid.x     = argv[0]->i;
	e->grid.y     = argv[1]->i;
	e->event_type = argv[2]->i & 1;

	self->have_event = 1;
	return 0;
}

/**
 * public
 */

static int proto_osc_mode(monome_t *monome, monome_mode_t mode) {
	SELF_FROM(monome);

	/* sys message?  why? */
	return lo_send_from(self->outgoing, self->server, LO_TT_IMMEDIATE, "/sys/mode", "i", mode);
}

static int proto_osc_led_set(monome_t *monome, uint_t x, uint_t y, uint_t on) {
	SELF_FROM(monome);
	return LO_SEND_MSG(set, "iii", x, y, !!on);
}

static int proto_osc_led_all(monome_t *monome, uint_t status) {
	SELF_FROM(monome);
	return LO_SEND_MSG(all, "i", status);
}

static int proto_osc_led_row(monome_t *monome, uint_t x_off, uint_t y,
                             size_t count, const uint8_t *data) {
	SELF_FROM(monome);

	if( count == 1 )
		return LO_SEND_MSG(row, "iii", x_off, y, data[0]);

	return LO_SEND_MSG(row, "iiii", x_off, y, data[0], data[1]);
}

static int proto_osc_led_col(monome_t *monome, uint_t x, uint_t y_off,
                             size_t count, const uint8_t *data) {
	SELF_FROM(monome);

	if( count == 1 )
		return LO_SEND_MSG(col, "iii", x, y_off, data[0]);

	return LO_SEND_MSG(col, "iiii", x, y_off, data[0], data[1]);
}

static int proto_osc_led_map(monome_t *monome, uint_t x_off, uint_t y_off,
                             const uint8_t *f) {
	SELF_FROM(monome);

	return LO_SEND_MSG(map, "iiiiiiiiii", x_off, y_off,
	                   f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7]);
}

static int proto_osc_led_intensity(monome_t *monome, uint_t brightness) {
	SELF_FROM(monome);
	return LO_SEND_MSG(intensity, "i", brightness);
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

	asprintf(&buf, "%s/grid/key", self->prefix);
	lo_server_add_method(self->server, buf, "iii", proto_osc_press_handler, self);
	m_free(buf);

#define cache_osc_path(base, path) asprintf(&self->base##_str, "%s/" path, self->prefix)
	cache_osc_path(set, "grid/led/set");
	cache_osc_path(all, "grid/led/all");
	cache_osc_path(map, "grid/led/map");
	cache_osc_path(col, "grid/led/col");
	cache_osc_path(row, "grid/led/row");
	cache_osc_path(intensity, "grid/led/intensity");
#undef cache_osc_path

	return 0;
}

static int proto_osc_close(monome_t *monome) {
	return 0;
}

static void proto_osc_free(monome_t *monome) {
	SELF_FROM(monome);

#define clear_osc_path(base) m_free(self->base##_str);
	clear_osc_path(set);
	clear_osc_path(all);
	clear_osc_path(map);
	clear_osc_path(col);
	clear_osc_path(row);
	clear_osc_path(intensity);
#undef clear_osc_path

	m_free(self->prefix);
	lo_server_free(self->server);
	lo_address_free(self->outgoing);

	self->prefix   = NULL;
	self->server   = NULL;
	self->outgoing = NULL;

	m_free(self);
}

monome_t *monome_protocol_new() {
	monome_osc_t *self = m_calloc(1, sizeof(monome_osc_t));
	monome_t *monome = (monome_t *) self;
	
	if( !monome )
		return NULL;
	
	monome->open       = proto_osc_open;
	monome->close      = proto_osc_close;
	monome->free       = proto_osc_free;

	monome->next_event = proto_osc_next_event;

	monome->mode       = proto_osc_mode;
	
	monome->led.set    = proto_osc_led_set;
	monome->led.all    = proto_osc_led_all;
	monome->led.map    = proto_osc_led_map;
	monome->led.row    = proto_osc_led_row;
	monome->led.col    = proto_osc_led_col;
	monome->led.intensity = proto_osc_led_intensity;

	monome->led_ring = NULL;

	return monome;
}
