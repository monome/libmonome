/*
 * This file is part of libmonome.
 * libmonome is copyright 2007-2010 will light <visinin@gmail.com>
 *
 * libmonome is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 *
 */

#define _GNU_SOURCE

#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "monome.h"
#include "monome_internal.h"
#include "monome_platform.h"

#ifndef LIBSUFFIX
#define LIBSUFFIX ".so"
#endif

#ifndef LIBDIR
#define LIBDIR "/usr/lib"
#endif

extern monome_cable_impl_t rotate[4];

/**
 * public
 */

monome_t *monome_init(const char *proto) {
	void *protocol_lib;
	monome_t *(*monome_protocol_new)();
	monome_t *monome;
	char *buf;
	
	if( asprintf(&buf, "%s/monome/protocol_%s%s", LIBDIR, proto, LIBSUFFIX) < 0 )
		return NULL;

	protocol_lib = dlopen(buf, RTLD_NOW);
	free(buf);
	
	if( !protocol_lib ) {
		fprintf(stderr, "couldn't load monome protocol module.  dlopen said:\n\t%s\n\n"
				"please make sure that libmonome is installed correctly!\n", dlerror());
		return NULL;
	}
	
	monome_protocol_new = dlsym(protocol_lib, "monome_protocol_new");
	
	if( !monome_protocol_new ) {
		fprintf(stderr, "couldn't initialize monome protocol module. dlopen said:\n\t%s\n\n"
				"please make sure you're using a valid protocol library!\n"
				"if this is a protocol library you wrote, make sure you're providing a \e[1mmonome_protocol_new\e[0m function.\n", dlerror());
		return NULL;
	}
	
	monome = (*monome_protocol_new)();
	
	if( !monome )
		return NULL;

	return monome;
}

monome_t *monome_open(const char *dev, const char *proto, ...) {
	monome_t *monome = monome_init(proto);
	va_list arguments;
	int error;
	
	if( !monome )
		return NULL;
	
	va_start(arguments, proto);
	error = monome->open(monome, dev, arguments);
	va_end(arguments);

	if( error ) {
		monome->free(monome);
		return NULL;
	}
	
	return monome;
}

void monome_close(monome_t *monome) {
	monome->close(monome);

	if( monome->serial )
		free(monome->serial);

	if( monome->device )
		free(monome->device);

	monome->free(monome);
}

void monome_set_orientation(monome_t *monome, monome_cable_t cable) {
	monome->cable = cable & MONOME_CABLE_TOP;
}

int monome_get_rows(monome_t *monome) {
	return ((monome->model >> 4) & 0xF) + 1;
}

int monome_get_cols(monome_t *monome) {
	return (monome->model & 0xF) + 1;
}

void monome_register_handler(monome_t *monome, unsigned int event_type, monome_callback_function_t cb, void *data) {
	monome_callback_t *handler;
	
	if( event_type > 2 || !cb )
		return;
	
	handler       = &monome->handlers[event_type];
	handler->cb   = cb;
	handler->data = data;
}

void monome_unregister_handler(monome_t *monome, unsigned int event_type) {
	monome_register_handler(monome, event_type, NULL, NULL);
}

int monome_next_event(monome_t *monome) {
	monome_callback_t *handler;
	monome_event_t e;

	struct timeval timeout = {0, 0};
	int ret;

	fd_set fds;

	FD_ZERO(&fds);
	FD_SET(monome->fd, &fds);

	ret = select(monome->fd + 1, &fds, NULL, NULL, &timeout);

	if( ret < 0 ) {
		perror("libmonome: error in select()");
		return -1;
	} else if( !ret )
		return -1;

	e.monome = monome;

	if( monome->next_event(monome, &e) )
		return 1;

	handler = &monome->handlers[e.event_type];

	if( !handler->cb )
		return 1;

	handler->cb(&e, handler->data);

	return 0;
}

void monome_main_loop(monome_t *monome) {
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

		if( monome->next_event(monome, &e) )
			continue;

		handler = &monome->handlers[e.event_type];
		if( handler->cb )
			handler->cb(&e, handler->data);
	} while( 1 );
}

int monome_clear(monome_t *monome, monome_clear_status_t status) {
	return monome->clear(monome, status);
}

int monome_intensity(monome_t *monome, unsigned int brightness) {
	return monome->intensity(monome, brightness);
}

int monome_mode(monome_t *monome, monome_mode_t mode) {
	return monome->mode(monome, mode);
}

int monome_led_on(monome_t *monome, unsigned int x, unsigned int y) {
	return rotate[monome->cable].led_on(monome, x, y);
}

int monome_led_off(monome_t *monome, unsigned int x, unsigned int y) {
	return rotate[monome->cable].led_off(monome, x, y);
}

int monome_led_col_8(monome_t *monome, unsigned int col, unsigned int *col_data) {
	return rotate[monome->cable].led_col_8(monome, col, col_data);
}

int monome_led_row_8(monome_t *monome, unsigned int row, unsigned int *row_data) {
	return rotate[monome->cable].led_row_8(monome, row, row_data);
}

int monome_led_col_16(monome_t *monome, unsigned int col, unsigned int *col_data) {
	return rotate[monome->cable].led_col_16(monome, col, col_data);
}

int monome_led_row_16(monome_t *monome, unsigned int row, unsigned int *row_data) {
	return rotate[monome->cable].led_row_16(monome, row, row_data);
}

int monome_led_frame(monome_t *monome, unsigned int quadrant, unsigned int *frame_data) {
	return rotate[monome->cable].led_frame(monome, quadrant, frame_data);
}
