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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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
                                      uint_t address, const uint8_t *data) {
	uint8_t buf[2] = {0, 0};
	uint_t xaddress = address;

	/* I guess this is a bit of a hack...but damn does it work well!

	   treating the address as a coordinate pair with itself lets us calculate
	   the row/col translation in one call using the existing rotation code
	   then we just pick whether we want the x coord (row) or y coord (col)
	   depending on what sort of message it is. */

	ROTATE_COORDS(monome, xaddress, address);

	switch( mode ) {
	case PROTO_SERIES_LED_ROW_8:
		if( ROTSPEC(monome).flags & ROW_COL_SWAP )
			address = xaddress;

		if( ROTSPEC(monome).flags & ROW_REVBITS )
			buf[1] = REVERSE_BYTE(*data);
		else
			buf[1] = *data;

		break;

	case PROTO_SERIES_LED_COL_8:
		if( !(ROTSPEC(monome).flags & ROW_COL_SWAP) )
			address = xaddress;

		if( ROTSPEC(monome).flags & COL_REVBITS )
			buf[1] = REVERSE_BYTE(*data);
		else
			buf[1] = *data;

		break;

	default:
		return -1;
	}

	if( ROTSPEC(monome).flags & ROW_COL_SWAP )
		mode = (!(mode - PROTO_SERIES_LED_ROW_8) << 4) + PROTO_SERIES_LED_ROW_8;

	buf[0] = mode | (address & 0x0F );

	return monome_write(monome, buf, sizeof(buf));
}

static int proto_series_led_col_row_16(monome_t *monome, proto_series_message_t mode, uint_t address, const uint8_t *data) {
	uint8_t buf[3] = {0, 0, 0};
	uint_t xaddress = address;

	ROTATE_COORDS(monome, xaddress, address);

	switch( mode ) {
	case PROTO_SERIES_LED_ROW_16:
		if( ROTSPEC(monome).flags & ROW_COL_SWAP )
			address = xaddress;

#ifndef LM_BIG_ENDIAN
		if( ROTSPEC(monome).flags & ROW_REVBITS ) {
			buf[1] = REVERSE_BYTE(data[1]);
			buf[2] = REVERSE_BYTE(data[0]);
		} else {
			buf[1] = data[0];
			buf[2] = data[1];
		}
#else
		if( ROTSPEC(monome).flags & ROW_REVBITS ) {
			buf[2] = REVERSE_BYTE(data[1]);
			buf[1] = REVERSE_BYTE(data[0]);
		} else {
			buf[2] = data[0];
			buf[1] = data[1];
		}
#endif

		break;

	case PROTO_SERIES_LED_COL_16:
		if( !(ROTSPEC(monome).flags & ROW_COL_SWAP) )
			address = xaddress;

#ifndef LM_BIG_ENDIAN
		if( ROTSPEC(monome).flags & COL_REVBITS ) {
			buf[1] = REVERSE_BYTE(data[1]);
			buf[2] = REVERSE_BYTE(data[0]);
		} else {
			buf[1] = data[0];
			buf[2] = data[1];
		}
#else
		if( ROTSPEC(monome).flags & COL_REVBITS ) {
			buf[2] = REVERSE_BYTE(data[1]);
			buf[1] = REVERSE_BYTE(data[0]);
		} else {
			buf[2] = data[0];
			buf[1] = data[1];
		}
#endif

		break;

	default:
		return -1;
	}

	if( ROTSPEC(monome).flags & ROW_COL_SWAP )
		mode = (!(mode - PROTO_SERIES_LED_ROW_16) << 4) + PROTO_SERIES_LED_ROW_16;

	buf[0] = mode | (address & 0x0F );

	return monome_write(monome, buf, sizeof(buf));
}

/**
 * led functions
 */

static int proto_series_led_all(monome_t *monome, uint_t status) {
	uint8_t buf = PROTO_SERIES_CLEAR | (status & 0x01);
	return monome_write(monome, &buf, sizeof(buf));
}

static int proto_series_led_intensity(monome_t *monome, uint_t brightness) {
	uint8_t buf = PROTO_SERIES_INTENSITY | (brightness & 0x0F);
	return monome_write(monome, &buf, sizeof(buf));
}

static int proto_series_led_set(monome_t *monome, uint_t x, uint_t y, uint_t on) {
	uint8_t buf[2];

	ROTATE_COORDS(monome, x, y);

	buf[0] = PROTO_SERIES_LED_ON + (!on << 4);
	buf[1] = (x << 4) | y;

	return monome_write(monome, buf, sizeof(buf));
}

static int proto_series_led_col(monome_t *monome, uint_t x, uint_t y_off,
                                size_t count, const uint8_t *data) {
	uint16_t sdata;

	switch( ((monome_get_rows(monome) > 8) << 1) | (count > 1)) {
	case 0x0: /* 1-byte monome, 1-byte message */
	case 0x1: /* 1-byte monome, 2-byte message */
		return proto_series_led_col_row_8(
			monome, PROTO_SERIES_LED_COL_8, x, data);

	case 0x2: /* 2-byte monome, 1-byte message */
		sdata = *data;

		return proto_series_led_col_row_16(
			monome, PROTO_SERIES_LED_COL_16, x,
			((const uint8_t *) &sdata));

	case 0x3: /* 2-byte monome, 2-byte message */
		return proto_series_led_col_row_16(
			monome, PROTO_SERIES_LED_COL_16, x, data);
	}

	return -1;
}

static int proto_series_led_row(monome_t *monome, uint_t x_off, uint_t y,
                                size_t count, const uint8_t *data) {
	uint16_t sdata;

	switch( ((monome_get_cols(monome) > 8) << 1) | (count > 1)) {
	case 0x0: /* 1-byte monome, 1-byte message */
	case 0x1: /* 1-byte monome, 2-byte message */
		return proto_series_led_col_row_8(
			monome, PROTO_SERIES_LED_ROW_8, y, data);

	case 0x2: /* 2-byte monome, 1-byte message */
		sdata = *data;

		return proto_series_led_col_row_16(
			monome, PROTO_SERIES_LED_ROW_16, y,
			((const uint8_t *) &sdata));

	case 0x3: /* 2-byte monome, 2-byte message */
		return proto_series_led_col_row_16(
			monome, PROTO_SERIES_LED_ROW_16, y, data);
	}

	return -1;
}

static int proto_series_led_map(monome_t *monome, uint_t x_off, uint_t y_off,
                                const uint8_t *data) {
	uint8_t buf[9];
	uint_t quadrant;

	memcpy(&buf[1], data, 8);
	ROTSPEC(monome).map_cb(monome, &buf[1]);

	ROTATE_COORDS(monome, x_off, y_off);
	quadrant = (x_off / 8) + ((y_off / 8) * 2);

	buf[0] = PROTO_SERIES_LED_FRAME | (quadrant & 0x03);

	return monome_write(monome, buf, sizeof(buf));
}

static monome_led_functions_t proto_series_led_functions = {
	.set = proto_series_led_set,
	.all = proto_series_led_all,
	.map = proto_series_led_map,
	.row = proto_series_led_row,
	.col = proto_series_led_col,
	.intensity = proto_series_led_intensity
};

/**
 * tilt functions
 *
 * see http://post.monome.org/comments.php?DiscussionID=773#Item_5
 */

static int proto_series_tilt_enable(monome_t *monome, uint_t sensor) {
	uint8_t buf[1] = {193};
	return monome_write(monome, buf, sizeof(buf));
}

static int proto_series_tilt_disable(monome_t *monome, uint_t sensor) {
	uint8_t buf[1] = {192};
	return monome_write(monome, buf, sizeof(buf));
}

static monome_tilt_functions_t proto_series_tilt_functions = {
	.enable = proto_series_tilt_enable,
	.disable = proto_series_tilt_disable
};

/**
 * module interface
 */

static int proto_series_next_event(monome_t *monome, monome_event_t *e) {
	uint8_t buf[2] = {0, 0};
	ssize_t read;

	read = monome_platform_read(monome, buf, sizeof(buf));
	if (read < sizeof(buf))
		return read;

	switch( buf[0] ) {
	case PROTO_SERIES_BUTTON_DOWN:
	case PROTO_SERIES_BUTTON_UP:
		e->event_type = (buf[0] == PROTO_SERIES_BUTTON_DOWN) ? MONOME_BUTTON_DOWN : MONOME_BUTTON_UP;
		e->grid.x = buf[1] >> 4;
		e->grid.y = buf[1] & 0x0F;

		UNROTATE_COORDS(monome, e->grid.x, e->grid.y);
		return 1;

	case PROTO_SERIES_TILT:
		SERIES_T(monome)->tilt.x = buf[1];
		goto tilt_common; /* shut up okay */

	case PROTO_SERIES_TILT + 1:
		SERIES_T(monome)->tilt.y = buf[1];

tilt_common: /* I SAID SHUT UP */
		e->event_type = MONOME_TILT;
		e->tilt.sensor = 0;
		e->tilt.x = SERIES_T(monome)->tilt.x;
		e->tilt.y = SERIES_T(monome)->tilt.y;
		e->tilt.z = 0;
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
	monome->rows = m->dimensions.rows;
	monome->cols = m->dimensions.cols;
	monome->serial = serial;
	monome->friendly = m->friendly;

	return monome_platform_open(monome, m, dev);
}

static int proto_series_close(monome_t *monome) {
	return monome_platform_close(monome);
}

static void proto_series_free(monome_t *monome) {
	m_free(monome);
}

#if defined(EMBED_PROTOS)
monome_t *monome_protocol_series_new(void) {
#else
monome_t *monome_protocol_new(void) {
#endif
	monome_t *monome = m_calloc(1, sizeof(series_t));

	if( !monome )
		return NULL;

	monome->open = proto_series_open;
	monome->close = proto_series_close;
	monome->free = proto_series_free;
	monome->next_event = proto_series_next_event;

	monome->led = &proto_series_led_functions;
	monome->led_level = NULL;
	monome->led_ring = NULL;
	monome->tilt = &proto_series_tilt_functions;

	SERIES_T(monome)->tilt.x = 0;
	SERIES_T(monome)->tilt.y = 0;

	return monome;
}
