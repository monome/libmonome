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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <monome.h>
#include "internal.h"
#include "platform.h"
#include "rotation.h"

#ifndef LIBSUFFIX
#define LIBSUFFIX ".so"
#endif

#ifndef LIBDIR
#define LIBDIR "/usr/lib"
#endif

#define DEFAULT_PROTOCOL "40h"

static monome_devmap_t mapping[] = {
	{"m64-%d",  "series", {8, 8},   "monome 64" , NO_QUIRKS},
	{"m128-%d", "series", {16, 8},  "monome 128", NO_QUIRKS},
	{"m256-%d", "series", {16, 16}, "monome 256", NO_QUIRKS},

	{"mk%d",    "series", {0, 0},   "monome kit", NO_QUIRKS},

	{"m40h%d",  "40h",    {8, 8},   "monome 40h", NO_QUIRKS},
	{"a40h-%d", "40h",    {8, 8},   "arduinome" , QUIRK_57600_BAUD},

	/* determine device dimensions in initialization */
	{"m%d",     "mext",   {0, 0},   "monome i2c", NO_QUIRKS},

	{NULL}
};

/**
 * private
 */

static monome_devmap_t *map_serial_to_device(const char *serial) {
	monome_devmap_t *m;
	int serialnum;

	for( m = mapping; m->sermatch; m++ )
		if( sscanf(serial, m->sermatch, &serialnum) )
			return m;

	return NULL;
}

/**
 * public
 */

monome_t *monome_open(const char *dev, ...) {
	monome_t *monome;
	monome_devmap_t *m;

	va_list arguments;
	char *serial, *proto;
	int error;

	if( !dev )
		return NULL;

	serial = NULL;
	m = NULL;

	/* first let's figure out which protocol to use */
	if( !strstr(dev, "://") ) {
		/* assume that the device is a tty...let's probe and see what device
		   we're dealing with */

		if( !(serial = monome_platform_get_dev_serial(dev)) )
			return NULL;

		if( (m = map_serial_to_device(serial)) )
			proto = m->proto;
		else
			return NULL;
	} else
		/* otherwise, we'll assume that what we have is an OSC URL.

		   in the future, maybe we'll have more protocol modules...something
		   to think about. */
		proto = "osc";

	if( !(monome = monome_platform_load_protocol(proto)) )
		goto err_init;

	va_start(arguments, dev);
	error = monome->open(monome, dev, serial, m, arguments);
	va_end(arguments);

	if( error )
		goto err_init;

	if( !(monome->device = m_strdup(dev)) )
		goto err_nomem;

	monome->rotation = MONOME_ROTATE_0;
	return monome;

err_nomem:
	monome->free(monome);

err_init:
	if( serial ) m_free(serial);
	return NULL;
}

void monome_close(monome_t *monome) {
	assert(monome);

	if( monome->serial )
		m_free((char *) monome->serial);

	if( monome->device )
		m_free((char *) monome->device);

	monome->close(monome);
	monome_platform_free(monome);
}

const char *monome_get_serial(monome_t *monome) {
	return monome->serial;
}

const char *monome_get_devpath(monome_t *monome) {
	return monome->device;
}

const char *monome_get_friendly_name(monome_t *monome) {
	return monome->friendly;
}

int monome_get_rows(monome_t *monome) {
	if( ROTSPEC(monome).flags & ROW_COL_SWAP )
		return monome->cols;
	else
		return monome->rows;
}

int monome_get_cols(monome_t *monome) {
	if( ROTSPEC(monome).flags & ROW_COL_SWAP )
		return monome->rows;
	else
		return monome->cols;
}

monome_rotate_t monome_get_rotation(monome_t *monome) {
	return monome->rotation;
}

void monome_set_rotation(monome_t *monome, monome_rotate_t rotation) {
	monome->rotation = rotation & 3;
}

int monome_register_handler(monome_t *monome, monome_event_type_t event_type,
                            monome_event_callback_t cb, void *data) {
	monome_callback_t *handler;

	if( event_type >= MONOME_EVENT_MAX )
		return EINVAL;

	handler       = &monome->handlers[event_type];
	handler->cb   = cb;
	handler->data = data;

	return 0;
}

int monome_unregister_handler(monome_t *monome,
                              monome_event_type_t event_type) {
	return monome_register_handler(monome, event_type, NULL, NULL);
}

int monome_event_next(monome_t *monome, monome_event_t *e) {
	e->monome = monome;

	if( !monome->next_event(monome, e) )
		return 0;
	return 1;
}

int monome_event_handle_next(monome_t *monome) {
	monome_callback_t *handler;
	monome_event_t e;

	if( !monome_event_next(monome, &e) )
		return 0;

	handler = &monome->handlers[e.event_type];

	if( !handler->cb )
		return 0;

	handler->cb(&e, handler->data);
	return 1;
}

int monome_get_fd(monome_t *monome) {
	return monome->fd;
}

int monome_mode(monome_t *monome, monome_mode_t mode) {
	return monome->mode(monome, mode);
}

int monome_led_set(monome_t *monome, uint_t x, uint_t y, uint_t on) {
	return monome->led.set(monome, x, y, on);
}

int monome_led_on(monome_t *monome, uint_t x, uint_t y) {
	return monome_led_set(monome, x, y, 1);
}

int monome_led_off(monome_t *monome, uint_t x, uint_t y) {
	return monome_led_set(monome, x, y, 0);
}

int monome_led_all(monome_t *monome, uint_t status) {
	return monome->led.all(monome, status);
}

int monome_led_map(monome_t *monome, uint_t x_off, uint_t y_off,
                   const uint8_t *data) {
	return monome->led.map(monome, x_off, y_off, data);
}

int monome_led_row(monome_t *monome, uint_t x_off, uint_t y,
				   size_t count, const uint8_t *data) {
	return monome->led.row(monome, x_off, y, count, data);
}

int monome_led_col(monome_t *monome, uint_t x, uint_t y_off,
				   size_t count, const uint8_t *data) {
	return monome->led.col(monome, x, y_off, count, data);
}

int monome_led_intensity(monome_t *monome, uint_t brightness) {
	return monome->led.intensity(monome, brightness);
}

int monome_led_level_set(monome_t *monome, uint_t x, uint_t y, uint_t level) {
	if( monome->led_level.set )
		return monome->led_level.set(monome, x, y, level);
	return -1;
}

int monome_led_level_all(monome_t *monome, uint_t level) {
	if( monome->led_level.all )
		return monome->led_level.all(monome, level);
	return -1;
}

int monome_led_level_map(monome_t *monome, uint_t x_off, uint_t y_off,
                         const uint8_t *data) {
	if( monome->led_level.map )
		return monome->led_level.map(monome, x_off, y_off, data);
	return -1;
}

int monome_led_level_row(monome_t *monome, uint_t x_off, uint_t y,
                         size_t count, const uint8_t *data) {
	if( monome->led_level.row )
		return monome->led_level.row(monome, x_off, y, count, data);
	return -1;
}

int monome_led_level_col(monome_t *monome, uint_t x, uint_t y_off,
                         size_t count, const uint8_t *data) {
	if( monome->led_level.col )
		return monome->led_level.col(monome, x, y_off, count, data);
	return -1;
}

int monome_led_ring_set(monome_t *monome, uint_t ring, uint_t led,
                        uint_t level) {
	if( !monome->led_ring )
		return -1;

	return monome->led_ring->set(monome, ring, led, level);
}

int monome_led_ring_all(monome_t *monome, uint_t ring, uint_t level) {
	if( !monome->led_ring )
		return -1;

	return monome->led_ring->all(monome, ring, level);
}

int monome_led_ring_map(monome_t *monome, uint_t ring, const uint8_t *levels) {
	if( !monome->led_ring )
		return -1;

	return monome->led_ring->map(monome, ring, levels);
}

int monome_led_ring_range(monome_t *monome, uint_t ring, uint_t start,
                          uint_t end, uint_t level) {
	if( !monome->led_ring )
		return -1;

	return monome->led_ring->range(monome, ring, start, end, level);
}
