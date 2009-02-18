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

#include <stdlib.h>
#include <stdint.h>

#include <monome.h>
#include "monome_internal.h"
#include "monome_platform.h"
#include "protocol_series.h"

/**
 * private
 */

static int monome_write(monome_t *monome, const uint8_t *buf, ssize_t bufsize) {
    if( monome_platform_write(monome, buf, bufsize) == bufsize )
		return 0;
	
	return -1;
}

static int proto_series_led_col_row(monome_t *monome, proto_series_message_t mode, unsigned int address, unsigned int *data) {
	uint8_t buf[3];
	
	switch( mode ) {
	case PROTO_SERIES_LED_COL_8:
	case PROTO_SERIES_LED_ROW_8:
		buf[0] = mode | (address & 0x0F );
		buf[1] = data[0];
		
		return monome_write(monome, buf, sizeof(buf) - sizeof(char));
		
	case PROTO_SERIES_LED_COL_16:
	case PROTO_SERIES_LED_ROW_16:
		buf[0] = mode | (address & 0x0F );
		buf[1] = data[0];
		buf[2] = data[1];
		
		return monome_write(monome, buf, sizeof(buf));
		
	default:
		break;
	}
	
	return -1;
}

static int proto_series_led(monome_t *monome, unsigned int status, unsigned int x, unsigned int y) {
	uint8_t buf[2];
	
	buf[0] = status;
	buf[1] = ( x << 4 ) | y;
	
	return monome_write(monome, buf, sizeof(buf));
}

/**
 * public
 */

int proto_series_clear(monome_t *monome, monome_clear_status_t status) {
	uint8_t buf = PROTO_SERIES_CLEAR | ( status & PROTO_SERIES_CLEAR_ON );
	return monome_write(monome, &buf, sizeof(buf));
}

int proto_series_intensity(monome_t *monome, unsigned int brightness) {
	uint8_t buf = PROTO_SERIES_INTENSITY | ( brightness & 0x0F );
	return monome_write(monome, &buf, sizeof(buf));
}

int proto_series_mode(monome_t *monome, monome_mode_t mode) {
	uint8_t buf = PROTO_SERIES_MODE | ( (mode & PROTO_SERIES_MODE_TEST) | (mode & PROTO_SERIES_MODE_SHUTDOWN) );
	return monome_write(monome, &buf, sizeof(buf));
}

int proto_series_led_on(monome_t *monome, unsigned int x, unsigned int y) {
	return proto_series_led(monome, PROTO_SERIES_LED_ON, x, y);
}

int proto_series_led_off(monome_t *monome, unsigned int x, unsigned int y) {
	return proto_series_led(monome, PROTO_SERIES_LED_OFF, x, y);
}

int proto_series_led_col_8(monome_t *monome, unsigned int col, unsigned int *col_data) {
	return proto_series_led_col_row(monome, PROTO_SERIES_LED_COL_8, col, col_data);
}

int proto_series_led_row_8(monome_t *monome, unsigned int row, unsigned int *row_data) {
	return proto_series_led_col_row(monome, PROTO_SERIES_LED_ROW_8, row, row_data);
}

int proto_series_led_col_16(monome_t *monome, unsigned int col, unsigned int *col_data) {
	return proto_series_led_col_row(monome, PROTO_SERIES_LED_COL_16, col, col_data);
}

int proto_series_led_row_16(monome_t *monome, unsigned int row, unsigned int *row_data) {
	return proto_series_led_col_row(monome, PROTO_SERIES_LED_ROW_16, row, row_data);
}

int proto_series_led_frame(monome_t *monome, unsigned int quadrant, unsigned int *frame_data) {
	unsigned int i;
	uint8_t buf[9];
	
	buf[0] = PROTO_SERIES_LED_FRAME | ( quadrant & 0x03 );
	
	for( i = 1; i < 9; i++ )
		if( !(buf[i] = *(frame_data++)) )
			return 0;
	
	return monome_write(monome, buf, sizeof(buf));
}

int proto_series_populate_event(monome_event_t *e, const uint8_t *buf, const ssize_t buf_size) {
	switch( buf[0] ) {
	case PROTO_SERIES_BUTTON_DOWN:
	case PROTO_SERIES_BUTTON_UP:
		e->event_type = (buf[0] == PROTO_SERIES_BUTTON_DOWN) ? MONOME_BUTTON_DOWN : MONOME_BUTTON_UP;
		e->x = buf[1] >> 4;
		e->y = buf[1] & 0x0F;
		return 0;
		
	case PROTO_SERIES_AUX_INPUT:
		/* soon */
		return 0;
	}
	
	return -1;
}

monome_t *monome_protocol_new() {
	monome_t *monome = calloc(1, sizeof(monome_t));
	
	if( !monome )
		return NULL;
	
	monome->populate_event = proto_series_populate_event;

	monome->clear      = proto_series_clear;
	monome->intensity  = proto_series_intensity;
	monome->mode       = proto_series_mode;
	
	monome->led_on     = proto_series_led_on;
	monome->led_off    = proto_series_led_off;
	monome->led_col_8  = proto_series_led_col_8;
	monome->led_row_8  = proto_series_led_row_8;
	monome->led_col_16 = proto_series_led_col_16;
	monome->led_row_16 = proto_series_led_row_16;
	monome->led_frame  = proto_series_led_frame;

	return monome;
}
