/*
 * This file is part of libmonome.
 * libmonome is copyright 2007, 2008 will light <visinin@gmail.com>
 *
 * libmonome is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <lo/lo.h>

#include "monome.h"
#include "monome_internal.h"
#include "protocol_osc.h"

#define SELF_FROM(what_okay) monome_osc_t *self = (monome_osc_t *) what_okay;
#define LO_SEND_MSG(type, ...) lo_send_from(self->outgoing, self->server, LO_TT_IMMEDIATE, self->type##_str, __VA_ARGS__)

int proto_osc_close(monome_t *monome);
void proto_osc_free(monome_t *monome);

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
	e->event_type = argv[2]->i;

	self->have_event = 1;
	return 0;
}

/**
 * public
 */

int proto_osc_clear(monome_t *monome, monome_clear_status_t status) {
	SELF_FROM(monome);
	return LO_SEND_MSG(clear, "i", status);
}

int proto_osc_intensity(monome_t *monome, unsigned int brightness) {
	SELF_FROM(monome);
	return LO_SEND_MSG(intensity, "i", brightness);
}

int proto_osc_mode(monome_t *monome, monome_mode_t mode) {
	SELF_FROM(monome);
	return LO_SEND_MSG(mode, "i", mode);
}

int proto_osc_led_on(monome_t *monome, unsigned int x, unsigned int y) {
	SELF_FROM(monome);
	return LO_SEND_MSG(led, "iii", x, y, 1);
}

int proto_osc_led_off(monome_t *monome, unsigned int x, unsigned int y) {
	SELF_FROM(monome);
	return LO_SEND_MSG(led, "iii", x, y, 0);
}

int proto_osc_led_col_8(monome_t *monome, unsigned int col, unsigned int *col_data) {
	SELF_FROM(monome);
	return LO_SEND_MSG(led_col, "ii", col, col_data[0]);
}

int proto_osc_led_row_8(monome_t *monome, unsigned int row, unsigned int *row_data) {
	SELF_FROM(monome);
	return LO_SEND_MSG(led_row, "ii", row, row_data[0]);
}

int proto_osc_led_col_16(monome_t *monome, unsigned int col, unsigned int *col_data) {
	SELF_FROM(monome);
	return LO_SEND_MSG(led_col, "iii", col, col_data[0], col_data[1]);
}

int proto_osc_led_row_16(monome_t *monome, unsigned int row, unsigned int *row_data) {
	SELF_FROM(monome);
	return LO_SEND_MSG(led_row, "iii", row, row_data[0], row_data[1]);
}

int proto_osc_led_frame(monome_t *monome, unsigned int quadrant, unsigned int *f) {
	SELF_FROM(monome);

	/* there has to be a cleaner way to do this */
	return LO_SEND_MSG(frame, "iiiiiiiii", f[0], f[1], f[2], f[3], f[4], f[5], f[6], f[7], quadrant);
}

int proto_osc_next_event(monome_t *monome, monome_event_t *e) {
	SELF_FROM(monome);

	self->e_ptr = e;
	self->have_event = 0;

	lo_server_recv_noblock(self->server, 0);

	return !self->have_event;
}

int proto_osc_open(monome_t *monome, const char *dev, va_list args) {
	SELF_FROM(monome);
	char *port, *buf;

	port = va_arg(args, char *);

	self->prefix   = lo_url_get_path(dev);
	self->server   = lo_server_new(port, proto_osc_lo_error);
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

int proto_osc_close(monome_t *monome) {
	return 0;
}

void proto_osc_free(monome_t *monome) {
	SELF_FROM(monome);

	free(self->prefix);
	lo_server_free(self->server);
	lo_address_free(self->outgoing);

	self->prefix   = NULL;
	self->server   = NULL;
	self->outgoing = NULL;
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
	monome->led_col_8  = proto_osc_led_col_8;
	monome->led_row_8  = proto_osc_led_row_8;
	monome->led_col_16 = proto_osc_led_col_16;
	monome->led_row_16 = proto_osc_led_row_16;
	monome->led_frame  = proto_osc_led_frame;

	return monome;
}
