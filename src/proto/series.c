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

static int proto_series_led(monome_t *monome, uint x, uint y, uint on) {
	uint8_t buf[2];

	ROTATE_COORDS(monome, x, y);

	buf[0] = PROTO_SERIES_LED_ON + (!on << 4);
	buf[1] = (x << 4) | y;

	return monome_write(monome, buf, sizeof(buf));
}

static int proto_series_led_col(monome_t *monome, uint col, size_t count, const uint8_t *data) {
	uint16_t sdata;

	switch( ((monome->cols > 8) << 1) | (count > 1)) {
	case 0x0: /* 1-byte monome, 1-byte message */
	case 0x1: /* 1-byte monome, 2-byte message */
		return proto_series_led_col_row_8(
			monome, PROTO_SERIES_LED_COL_8, col, data);

	case 0x2: /* 2-byte monome, 1-byte message */
		sdata = *data;

		return proto_series_led_col_row_16(
			monome, PROTO_SERIES_LED_COL_16, col,
			((const uint8_t *) &sdata));

	case 0x3: /* 2-byte monome, 2-byte message */
		return proto_series_led_col_row_16(
			monome, PROTO_SERIES_LED_COL_16, col, data);
	}

	return -1;
}

static int proto_series_led_row(monome_t *monome, uint row, size_t count, const uint8_t *data) {
	uint16_t sdata;

	switch( ((monome->rows > 8) << 1) | (count > 1)) {
	case 0x0: /* 1-byte monome, 1-byte message */
	case 0x1: /* 1-byte monome, 2-byte message */
		return proto_series_led_col_row_8(
			monome, PROTO_SERIES_LED_ROW_8, row, data);

	case 0x2: /* 2-byte monome, 1-byte message */
		sdata = *data;

		return proto_series_led_col_row_16(
			monome, PROTO_SERIES_LED_ROW_16, row,
			((const uint8_t *) &sdata));

	case 0x3: /* 2-byte monome, 2-byte message */
		return proto_series_led_col_row_16(
			monome, PROTO_SERIES_LED_ROW_16, row, data);
	}

	return -1;
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

	monome->led        = proto_series_led;
	monome->led_col    = proto_series_led_col;
	monome->led_row    = proto_series_led_row;
	monome->led_frame  = proto_series_led_frame;

	return monome;
}
