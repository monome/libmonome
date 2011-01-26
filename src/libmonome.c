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

#define DEFAULT_MODEL    MONOME_DEVICE_40h
#define DEFAULT_PROTOCOL "40h"

static monome_devmap_t mapping[] = {
	{"m64-%d",  "series", {8, 8},   "monome 64" },
	{"m128-%d", "series", {16, 8},  "monome 128"},
	{"m256-%d", "series", {16, 16}, "monome 256"},

	/* need specialized protocol module
	   also, how do we determine dimensions? */
	{"mk%d",    "series", {8, 8},   "monome kit"},

	{"m40h%d",  "40h",    {8, 8},   "monome 40h"},
	{"a40h-%d", "40h",    {8, 8},   "arduinome" },

	/* determine device dimensions in initialization */
	{"m%d",     "mext",   {0, 0},   "monome i2c"},

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
	if( *dev == '/' ) {
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

	if( !(monome->device = strdup(dev)) )
		goto err_nomem;

	monome->rotation = MONOME_ROTATE_0;
	return monome;

err_nomem:
	monome->free(monome);

err_init:
	if( serial ) free(serial);
	return NULL;
}

void monome_close(monome_t *monome) {
	assert(monome);

	if( monome->serial )
		free((char *) monome->serial);

	if( monome->device )
		free((char *) monome->device);

	monome->close(monome);
	monome_platform_free(monome);
}

const char *monome_get_serial(monome_t *monome) {
	return monome->serial;
}

const char *monome_get_devpath(monome_t *monome) {
	return monome->device;
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

	if( event_type > 2 )
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

int monome_clear(monome_t *monome, monome_clear_status_t status) {
	return monome->clear(monome, status);
}

int monome_intensity(monome_t *monome, uint_t brightness) {
	return monome->intensity(monome, brightness);
}

int monome_mode(monome_t *monome, monome_mode_t mode) {
	return monome->mode(monome, mode);
}

int monome_led(monome_t *monome, uint_t x, uint_t y, uint_t on) {
	return monome->led(monome, x, y, on);
}

int monome_led_on(monome_t *monome, uint_t x, uint_t y) {
	return monome_led(monome, x, y, 1);
}

int monome_led_off(monome_t *monome, uint_t x, uint_t y) {
	return monome_led(monome, x, y, 0);
}

int monome_led_col(monome_t *monome, uint_t col,
				   size_t count, const uint8_t *data) {
	return monome->led_col(monome, col, count, data);
}

int monome_led_row(monome_t *monome, uint_t row,
				   size_t count, const uint8_t *data) {
	return monome->led_row(monome, row, count, data);
}

int monome_led_frame(monome_t *monome, uint_t x_off, uint_t y_off,
                     const uint8_t *frame_data) {
	return monome->led_frame(monome, x_off, y_off, frame_data);
}
