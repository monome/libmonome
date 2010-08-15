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

#include <stdlib.h>
#include <stdint.h>

#include <monome.h>
#include "internal.h"
#include "platform.h"
#include "rotation.h"

#include "series.h"

/**
 * private
 */

static int monome_write(monome_t *monome, const uint8_t *buf,
						ssize_t bufsize) {
    if( monome_platform_write(monome, buf, bufsize) == bufsize )
		return 0;

	return -1;
}

static int proto_series_led_col_row_8(monome_t *monome,
									  proto_series_message_t mode,
									  uint address, const uint8_t *data) {
	uint8_t buf[2] = {0, 0};
	uint xaddress = address;

	/* I guess this is a bit of a hack...but damn does it work well!

	   treating the address as a coordinate pair with itself lets us calculate
	   the row/col translation in one call using the existing rotation code
	   then we just pick whether we want the x coord (row) or y coord (col)
	   depending on what sort of message it is. */

	ROTATE_COORDS(monome, xaddress, address);

	switch( mode ) {
	case PROTO_SERIES_LED_ROW_8:
		address = xaddress;

		if( ORIENTATION(monome).flags & ROW_REVBITS )
			buf[1] = REVERSE_BYTE(*data);
		else
			buf[1] = *data;

		break;

	case PROTO_SERIES_LED_COL_8:
		if( ORIENTATION(monome).flags & COL_REVBITS )
			buf[1] = REVERSE_BYTE(*data);
		else
			buf[1] = *data;

		break;
	
	default:
		return -1;
	}

	if( ORIENTATION(monome).flags & ROW_COL_SWAP )
		mode = (!(mode - PROTO_SERIES_LED_ROW_8) << 4) + PROTO_SERIES_LED_ROW_8;

	buf[0] = mode | (address & 0x0F );

	return monome_write(monome, buf, sizeof(buf));
}

static int proto_series_led_col_row_16(monome_t *monome, proto_series_message_t mode, uint address, const uint8_t *data) {
	uint8_t buf[3] = {0, 0, 0};
	uint xaddress = address;

	ROTATE_COORDS(monome, xaddress, address);

	switch( mode ) {
	case PROTO_SERIES_LED_ROW_16:
		address = xaddress;

		if( ORIENTATION(monome).flags & ROW_REVBITS ) {
			buf[1] = REVERSE_BYTE(data[1]);
			buf[2] = REVERSE_BYTE(data[0]);
		} else {
			buf[1] = data[0];
			buf[2] = data[1];
		}

		break;

	case PROTO_SERIES_LED_COL_16:
		if( ORIENTATION(monome).flags & COL_REVBITS ) {
			buf[1] = REVERSE_BYTE(data[1]);
			buf[2] = REVERSE_BYTE(data[0]);
		} else {
			buf[1] = data[0];
			buf[2] = data[1];
		}

		break;

	default:
		return -1;
	}

	if( ORIENTATION(monome).flags & ROW_COL_SWAP )
		mode = (!(mode - PROTO_SERIES_LED_ROW_16) << 4) + PROTO_SERIES_LED_ROW_16;

	buf[0] = mode | (address & 0x0F );

	return monome_write(monome, buf, sizeof(buf));
}

static int proto_series_led(monome_t *monome, uint status, uint x, uint y) {
	uint8_t buf[2];

	ROTATE_COORDS(monome, x, y);

	buf[0] = status;
	buf[1] = (x << 4) | y;

	return monome_write(monome, buf, sizeof(buf));
}

/**
 * public
 */

static int proto_series_clear(monome_t *monome, monome_clear_status_t status) {
	uint8_t buf = PROTO_SERIES_CLEAR | (status & PROTO_SERIES_CLEAR_ON);
	return monome_write(monome, &buf, sizeof(buf));
}

static int proto_series_intensity(monome_t *monome, uint brightness) {
	uint8_t buf = PROTO_SERIES_INTENSITY | (brightness & 0x0F);
	return monome_write(monome, &buf, sizeof(buf));
}

static int proto_series_mode(monome_t *monome, monome_mode_t mode) {
	uint8_t buf = PROTO_SERIES_MODE | ((mode & PROTO_SERIES_MODE_TEST) | (mode & PROTO_SERIES_MODE_SHUTDOWN));
	return monome_write(monome, &buf, sizeof(buf));
}

static int proto_series_led_on(monome_t *monome, uint x, uint y) {
	return proto_series_led(monome, PROTO_SERIES_LED_ON, x, y);
}

static int proto_series_led_off(monome_t *monome, uint x, uint y) {
	return proto_series_led(monome, PROTO_SERIES_LED_OFF, x, y);
}

static int proto_series_led_col(monome_t *monome, uint col, size_t count, const uint8_t *data) {
	if( count == 1 )
		return proto_series_led_col_row_8(monome, PROTO_SERIES_LED_COL_8, col, data);
	return proto_series_led_col_row_16(monome, PROTO_SERIES_LED_COL_16, col, data);
}

static int proto_series_led_row(monome_t *monome, uint row, size_t count, const uint8_t *data) {
	if( count == 1 )
		return proto_series_led_col_row_8(monome, PROTO_SERIES_LED_ROW_8, row, data);
	return proto_series_led_col_row_16(monome, PROTO_SERIES_LED_ROW_16, row, data);
}

static int proto_series_led_frame(monome_t *monome, uint quadrant, const uint8_t *frame_data) {
	uint8_t buf[9];

	/* by treating frame_data as a bigger integer, we can copy it in
	   one or two operations (instead of 8) */
#ifdef __LP64__
	*((uint64_t *) &buf[1]) = *((uint64_t *) frame_data);
#else
	*((uint32_t *) &buf[1]) = *((uint32_t *) frame_data);
	*((uint32_t *) &buf[5]) = *(((uint32_t *) frame_data) + 1);
#endif

	ORIENTATION(monome).frame_cb(monome, &quadrant, &buf[1]);
	buf[0] = PROTO_SERIES_LED_FRAME | (quadrant & 0x03);

	return monome_write(monome, buf, sizeof(buf));
}

static int proto_series_next_event(monome_t *monome, monome_event_t *e) {
	uint8_t buf[2] = {0, 0};

	if( monome_platform_read(monome, buf, sizeof(buf)) < sizeof(buf) )
		return 0;

	switch( buf[0] ) {
	case PROTO_SERIES_BUTTON_DOWN:
	case PROTO_SERIES_BUTTON_UP:
		e->event_type = (buf[0] == PROTO_SERIES_BUTTON_DOWN) ? MONOME_BUTTON_DOWN : MONOME_BUTTON_UP;
		e->x = buf[1] >> 4;
		e->y = buf[1] & 0x0F;

		UNROTATE_COORDS(monome, e->x, e->y);
		return 1;

	case PROTO_SERIES_AUX_INPUT:
		/* soon */
		return 0;
	}

	return 0;
}

static int proto_series_open(monome_t *monome, const char *dev,
							 const char *serial, const monome_devmap_t *m,
							 va_list args) {
	monome->rows   = m->dimensions.rows;
	monome->cols   = m->dimensions.cols;
	monome->serial = serial;

	return monome_platform_open(monome, dev);
}

static int proto_series_close(monome_t *monome) {
	return monome_platform_close(monome);
}

static void proto_series_free(monome_t *monome) {
	free(monome);
}

monome_t *monome_protocol_new() {
	monome_t *monome = calloc(1, sizeof(monome_t));

	if( !monome )
		return NULL;

	monome->open       = proto_series_open;
	monome->close      = proto_series_close;
	monome->free       = proto_series_free;

	monome->next_event = proto_series_next_event;

	monome->clear      = proto_series_clear;
	monome->intensity  = proto_series_intensity;
	monome->mode       = proto_series_mode;

	monome->led_on     = proto_series_led_on;
	monome->led_off    = proto_series_led_off;
	monome->led_col    = proto_series_led_col;
	monome->led_row    = proto_series_led_row;
	monome->led_frame  = proto_series_led_frame;

	return monome;
}
