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
#include <stdlib.h>
#include <stdint.h>
#include <lo/lo.h>

#include <monome.h>
#include "platform.h"
#include "internal.h"

#include "osc.h"

#define SELF_FROM(what_okay) monome_osc_t *self = (monome_osc_t *) what_okay;
#define LO_SEND_MSG(type, ...) lo_send_from(self->outgoing, self->server, LO_TT_IMMEDIATE, self->type##_str, __VA_ARGS__)

#define OSC_HANDLER_FUNC(x)\
	static int x(const char *path, const char *types,\
				 lo_arg **argv, int argc,\
				 lo_message data, void *user_data)

static int proto_osc_close(monome_t *monome);
static void proto_osc_free(monome_t *monome);

/**
 * private
 */

static void proto_osc_lo_error(int num, const char *error_msg, const char *path) {
	fprintf(stderr, "libmonome: liblo server error %d in %s:\n\t%s\n", num, path, error_msg);
	fflush(stderr);
}

OSC_HANDLER_FUNC(proto_osc_press_handler) {
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

OSC_HANDLER_FUNC(proto_osc_delta_handler) {
	SELF_FROM(user_data);
	monome_event_t *e = self->e_ptr;

	if( !e )
		return 1;

	e->event_type = MONOME_ENCODER_DELTA;
	e->encoder.number = argv[0]->i;
	e->encoder.delta = argv[1]->i;

	self->have_event = 1;
	return 0;
}

OSC_HANDLER_FUNC(proto_osc_enc_key_handler) {
	SELF_FROM(user_data);
	monome_event_t *e = self->e_ptr;

	if( !e )
		return 1;

	if( argv[1]->i )
		e->event_type = MONOME_ENCODER_KEY_DOWN;
	else
		e->event_type = MONOME_ENCODER_KEY_UP;
	e->encoder.number = argv[0]->i;

	self->have_event = 1;
	return 0;
}

/**
 * led functions
 */

static int proto_osc_led_set(monome_t *monome, uint_t x, uint_t y, uint_t on) {
	SELF_FROM(monome);
	return LO_SEND_MSG(led_set, "iii", x, y, !!on);
}

static int proto_osc_led_all(monome_t *monome, uint_t status) {
	SELF_FROM(monome);
	return LO_SEND_MSG(led_all, "i", status);
}

static int proto_osc_led_row(monome_t *monome, uint_t x_off, uint_t y,
                             size_t count, const uint8_t *data) {
	SELF_FROM(monome);

	if( count == 1 )
		return LO_SEND_MSG(led_row, "iii", x_off, y, data[0]);

	return LO_SEND_MSG(led_row, "iiii", x_off, y, data[0], data[1]);
}

static int proto_osc_led_col(monome_t *monome, uint_t x, uint_t y_off,
                             size_t count, const uint8_t *data) {
	SELF_FROM(monome);

	if( count == 1 )
		return LO_SEND_MSG(led_col, "iii", x, y_off, data[0]);

	return LO_SEND_MSG(led_col, "iiii", x, y_off, data[0], data[1]);
}

static int proto_osc_led_map(monome_t *monome, uint_t x_off, uint_t y_off,
                             const uint8_t *f) {
	SELF_FROM(monome);

	return LO_SEND_MSG(led_map, "iiiiiiiiii", x_off, y_off,
	                   f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7]);
}

static int proto_osc_led_intensity(monome_t *monome, uint_t brightness) {
	SELF_FROM(monome);
	return LO_SEND_MSG(led_intensity, "i", brightness);
}

static monome_led_functions_t proto_osc_led_functions = {
	.set = proto_osc_led_set,
	.all = proto_osc_led_all,
	.map = proto_osc_led_map,
	.row = proto_osc_led_row,
	.col = proto_osc_led_col,
	.intensity = proto_osc_led_intensity
};

/**
 * led ring functions
 */

static int proto_osc_led_ring_set(monome_t *monome, uint_t ring, uint_t led,
                                  uint_t level) {
	SELF_FROM(monome);
	return LO_SEND_MSG(ring_set, "iii", ring, led, level);
}

static int proto_osc_led_ring_all(monome_t *monome, uint_t ring,
                                  uint_t level) {
	SELF_FROM(monome);
	return LO_SEND_MSG(ring_all, "ii", ring, level);
}

static int proto_osc_led_ring_map(monome_t *monome, uint_t ring,
                                  const uint8_t *levels) {
	SELF_FROM(monome);
	lo_message m;
	int i, ret;

	m = lo_message_new();

	lo_message_add_int32(m, ring);
	for( i = 0; i < 64; i++ )
		lo_message_add_int32(m, levels[i]);

	ret = lo_send_message_from(self->outgoing, self->server, self->ring_map_str, m);
	lo_message_free(m);

	return ret;
}

static int proto_osc_led_ring_range(monome_t *monome, uint_t ring,
                                    uint_t start, uint_t end, uint_t level) {
	SELF_FROM(monome);
	return LO_SEND_MSG(ring_range, "iiii", ring, start, end, level);
}

static monome_led_ring_functions_t proto_osc_led_ring_functions = {
	.set = proto_osc_led_ring_set,
	.all = proto_osc_led_ring_all,
	.map = proto_osc_led_ring_map,
	.range = proto_osc_led_ring_range
};

/**
 * module interface
 */

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

#define ASPRINTF_OR_BAIL(...) do { \
	if (asprintf(__VA_ARGS__) < 0) \
		return -1;                 \
	} while (0);

	ASPRINTF_OR_BAIL(&buf, "%s/grid/key", self->prefix);
	lo_server_add_method(self->server, buf, "iii", proto_osc_press_handler, self);
	m_free(buf);

	ASPRINTF_OR_BAIL(&buf, "%s/enc/delta", self->prefix);
	lo_server_add_method(self->server, buf, "ii", proto_osc_delta_handler, self);
	m_free(buf);

	ASPRINTF_OR_BAIL(&buf, "%s/enc/key", self->prefix);
	lo_server_add_method(self->server, buf, "ii", proto_osc_enc_key_handler, self);
	m_free(buf);

#define CACHE_OSC_PATH(base, path) ASPRINTF_OR_BAIL(&self->base##_str, "%s/" path, self->prefix)
	CACHE_OSC_PATH(led_set, "grid/led/set");
	CACHE_OSC_PATH(led_all, "grid/led/all");
	CACHE_OSC_PATH(led_map, "grid/led/map");
	CACHE_OSC_PATH(led_col, "grid/led/col");
	CACHE_OSC_PATH(led_row, "grid/led/row");
	CACHE_OSC_PATH(led_intensity, "grid/led/intensity");

	CACHE_OSC_PATH(ring_set, "ring/set");
	CACHE_OSC_PATH(ring_all, "ring/all");
	CACHE_OSC_PATH(ring_map, "ring/map");
	CACHE_OSC_PATH(ring_range, "ring/range");
#undef CACHE_OSC_PATH
#undef ASPRINTF_OR_BAIL

	return 0;
}

static int proto_osc_close(monome_t *monome) {
	return 0;
}

static void proto_osc_free(monome_t *monome) {
	SELF_FROM(monome);

#define clear_osc_path(base) m_free(self->base##_str);
	clear_osc_path(led_set);
	clear_osc_path(led_all);
	clear_osc_path(led_map);
	clear_osc_path(led_col);
	clear_osc_path(led_row);
	clear_osc_path(led_intensity);

	clear_osc_path(ring_set);
	clear_osc_path(ring_all);
	clear_osc_path(ring_map);
	clear_osc_path(ring_range);
#undef clear_osc_path

	m_free(self->prefix);
	lo_server_free(self->server);
	lo_address_free(self->outgoing);

	self->prefix   = NULL;
	self->server   = NULL;
	self->outgoing = NULL;

	m_free(self);
}

#if defined(EMBED_PROTOS)
monome_t *monome_protocol_osc_new(void) {
#else
monome_t *monome_protocol_new(void) {
#endif
	monome_osc_t *self = m_calloc(1, sizeof(monome_osc_t));
	monome_t *monome = (monome_t *) self;
	
	if( !monome )
		return NULL;
	
	monome->open       = proto_osc_open;
	monome->close      = proto_osc_close;
	monome->free       = proto_osc_free;

	monome->next_event = proto_osc_next_event;

	monome->led = &proto_osc_led_functions;
	monome->led_level = NULL;
	monome->led_ring = &proto_osc_led_ring_functions;

	return monome;
}
