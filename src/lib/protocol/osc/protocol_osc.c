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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <lo/lo.h>

#include "monome.h"
#include "monome_internal.h"
#include "protocol_osc.h"

int proto_osc_close(monome_t *monome);
void proto_osc_free(monome_t *monome);

/**
 * private
 */

static void proto_osc_lo_error(int num, const char *error_msg, const char *path) {
	fprintf(stderr, "libmonome: liblo server error %d in %s:\n\t%s\n", num, path, error_msg);
	fflush(stderr);
}

/**
 * public
 */

int proto_osc_clear(monome_t *monome, monome_clear_status_t status) {
	return 0;
}

int proto_osc_intensity(monome_t *monome, unsigned int brightness) {
	return 0;
}

int proto_osc_mode(monome_t *monome, monome_mode_t mode) {
	return 0;
}

int proto_osc_led_on(monome_t *monome, unsigned int x, unsigned int y) {
	return 0;
}

int proto_osc_led_off(monome_t *monome, unsigned int x, unsigned int y) {
	return 0;
}

int proto_osc_led_col_8(monome_t *monome, unsigned int col, unsigned int *col_data) {
	return 0;
}

int proto_osc_led_row_8(monome_t *monome, unsigned int row, unsigned int *row_data) {
	return 0;
}

int proto_osc_led_col_16(monome_t *monome, unsigned int col, unsigned int *col_data) {
	return 0;
}

int proto_osc_led_row_16(monome_t *monome, unsigned int row, unsigned int *row_data) {
	return 0;
}

int proto_osc_led_frame(monome_t *monome, unsigned int quadrant, unsigned int *frame_data) {
	return 0;
}

int proto_osc_next_event(monome_t *monome, monome_event_t *e) {
	return 0;
}

int proto_osc_open(monome_t *monome, const char *dev, va_list args) {
	monome_osc_t *self = (monome_osc_t *) monome;
	char *port = va_arg(args, char *);

	self->prefix   = lo_url_get_path(dev);
	self->server   = lo_server_new(port, proto_osc_lo_error);
	self->outgoing = lo_address_new_from_url(dev);

	if( (monome->fd = lo_server_get_socket_fd(self->server)) < 0 ) {
		proto_osc_close(monome);
		proto_osc_free(monome);
		return 1;
	}

	return 0;
}

int proto_osc_close(monome_t *monome) {
	return 0;
}

void proto_osc_free(monome_t *monome) {
	monome_osc_t *self = (monome_osc_t *) monome;

	free(self->prefix);
	lo_server_free(self->server);
	lo_address_free(self->outgoing);

	self->prefix   = NULL;
	self->server   = NULL;
	self->outgoing = NULL;
}

monome_t *monome_protocol_new() {
	monome_osc_t *mosc = calloc(1, sizeof(monome_osc_t));
	monome_t *monome = (monome_t *) mosc;
	
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
