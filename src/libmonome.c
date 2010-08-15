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

#include <assert.h>
#include <errno.h>
#include <dlfcn.h>
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

static void monome_free(monome_t *monome) {
	void *dl_handle = monome->dl_handle;

	monome->free(monome);
	dlclose(dl_handle);
}

/**
 * public
 */

monome_t *monome_init(const char *proto) {
	void *dl_handle;
	monome_t *(*monome_protocol_new)();
	monome_t *monome;
	char *buf;
	
	if( asprintf(&buf, "%s/monome/protocol_%s%s", LIBDIR, proto, LIBSUFFIX) < 0 )
		return NULL;

	dl_handle = dlopen(buf, RTLD_LAZY);
	free(buf);
	
	if( !dl_handle ) {
		fprintf(stderr, "couldn't load monome protocol module.  dlopen said:\n\t%s\n\n"
				"please make sure that libmonome is installed correctly!\n", dlerror());
		return NULL;
	}
	
	monome_protocol_new = dlsym(dl_handle, "monome_protocol_new");
	
	if( !monome_protocol_new ) {
		fprintf(stderr, "couldn't initialize monome protocol module. dlopen said:\n\t%s\n\n"
				"please make sure you're using a valid protocol library!\n"
				"if this is a protocol library you wrote, make sure you're providing a \e[1mmonome_protocol_new\e[0m function.\n", dlerror());
		goto err;
	}
	
	monome = (*monome_protocol_new)();
	
	if( !monome )
		goto err;

	monome->dl_handle = dl_handle;
	return monome;

err:
	dlclose(dl_handle);
	return NULL;
}

monome_t *monome_open(const char *dev, ...) {
	monome_t *monome;
	monome_devmap_t *m;

	va_list arguments;
	char *serial, *proto;
	int error;

	assert(dev);
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
			proto = DEFAULT_PROTOCOL;
	} else
		/* otherwise, we'll assume that what we have is an OSC URL.

		   in the future, maybe we'll have more protocol modules...something
		   to think about. */
		proto = "osc";

	if( !(monome = monome_init(proto)) )
		goto err_init;

	va_start(arguments, dev);
	error = monome->open(monome, dev, serial, m, arguments);
	va_end(arguments);

	if( error )
		goto err_init;

	if( !(monome->device = strdup(dev)) )
		goto err_nomem;

	monome->orientation = MONOME_CABLE_LEFT;
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
	monome_free(monome);
}

const char *monome_get_serial(monome_t *monome) {
	return monome->serial;
}

const char *monome_get_devpath(monome_t *monome) {
	return monome->device;
}

int monome_get_rows(monome_t *monome) {
	if( ORIENTATION(monome).flags & ROW_COL_SWAP )
		return monome->cols;
	else
		return monome->rows;
}

int monome_get_cols(monome_t *monome) {
	if( ORIENTATION(monome).flags & ROW_COL_SWAP )
		return monome->rows;
	else
		return monome->cols;
}

monome_cable_t monome_get_orientation(monome_t *monome) {
	return monome->orientation;
}

void monome_set_orientation(monome_t *monome, monome_cable_t cable) {
	monome->orientation = cable & 3;
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

void monome_event_loop(monome_t *monome) {
	monome_callback_t *handler;
	monome_event_t e;

	fd_set fds;

	e.monome = monome;

	do {
		FD_ZERO(&fds);
		FD_SET(monome->fd, &fds);

		if( select(monome->fd + 1, &fds, NULL, NULL, NULL) < 0 ) {
			perror("libmonome: error in select()");
			break;
		}

		if( !monome->next_event(monome, &e) )
			continue;

		handler = &monome->handlers[e.event_type];
		if( !handler->cb )
			continue;

		handler->cb(&e, handler->data);
	} while( 1 );
}

int monome_get_fd(monome_t *monome) {
	return monome->fd;
}

int monome_clear(monome_t *monome, monome_clear_status_t status) {
	return monome->clear(monome, status);
}

int monome_intensity(monome_t *monome, uint brightness) {
	return monome->intensity(monome, brightness);
}

int monome_mode(monome_t *monome, monome_mode_t mode) {
	return monome->mode(monome, mode);
}

int monome_led_on(monome_t *monome, uint x, uint y) {
	return monome->led_on(monome, x, y);
}

int monome_led_off(monome_t *monome, uint x, uint y) {
	return monome->led_off(monome, x, y);
}

int monome_led_col(monome_t *monome, uint col, size_t count, const uint8_t *data) {
	return monome->led_col(monome, col, count, data);
}

int monome_led_row(monome_t *monome, uint row, size_t count, const uint8_t *data) {
	return monome->led_row(monome, row, count, data);
}

int monome_led_frame(monome_t *monome, uint quadrant, const uint8_t *frame_data) {
	return monome->led_frame(monome, quadrant, frame_data);
}
