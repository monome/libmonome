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

#include "40h.h"

/**
 * private
 */

static int monome_write(monome_t *monome, const uint8_t *buf, ssize_t bufsize) {
	if( monome_platform_write(monome, buf, bufsize) == bufsize )
		return 0;

	return -1;
}

static int proto_40h_led_col_row(monome_t *monome, proto_40h_message_t mode, uint_t address, const uint8_t *data) {
	uint8_t buf[2];
	uint_t xaddress = address;

	ROTATE_COORDS(monome, xaddress, address);

	switch( mode ) {
	case PROTO_40h_LED_ROW:
		address = xaddress;

		if( ROTSPEC(monome).flags & ROW_REVBITS )
			buf[1] = REVERSE_BYTE(*data);
		else
			buf[1] = *data;

		break;

	case PROTO_40h_LED_COL:
		if( ROTSPEC(monome).flags & COL_REVBITS )
			buf[1] = REVERSE_BYTE(*data);
		else
			buf[1] = *data;

		break;

	default:
		return -1;
	}

	if( ROTSPEC(monome).flags & ROW_COL_SWAP )
		mode = (!(mode - PROTO_40h_LED_ROW) << 4) + PROTO_40h_LED_ROW;

	buf[0] = mode | (address & 0x7 );

	return monome_write(monome, buf, sizeof(buf));
}

/**
 * public
 */

static int proto_40h_led_all(monome_t *monome, uint_t status) {
	uint_t i;
	uint8_t buf[2] = {0, (status) ? 0xFF : 0};

	for( i = 0; i < 8; i++ ) {
		buf[0] = PROTO_40h_LED_ROW | (i & 0xF);
		monome_write(monome, buf, sizeof(buf));
	}

	return sizeof(buf) * i;
}

static int proto_40h_intensity(monome_t *monome, uint_t brightness) {
	uint8_t buf[2] = {PROTO_40h_INTENSITY, brightness};
	return monome_write(monome, buf, sizeof(buf));
}

static int proto_40h_led_set(monome_t *monome, uint_t x, uint_t y, uint_t on) {
	uint8_t buf[2];

	ROTATE_COORDS(monome, x, y);

	x &= 0x7;
	y &= 0x7;

	buf[0] = PROTO_40h_LED_OFF + !!on;
	buf[1] = (x << 4) | y;

	return monome_write(monome, buf, sizeof(buf));
}

static int proto_40h_led_col(monome_t *monome, uint_t x, uint_t y_off,
                             size_t count, const uint8_t *data) {
	return proto_40h_led_col_row(monome, PROTO_40h_LED_COL, x, data);
}

static int proto_40h_led_row(monome_t *monome, uint_t x_off, uint_t y,
                             size_t count, const uint8_t *data) {
	return proto_40h_led_col_row(monome, PROTO_40h_LED_ROW, y, data);
}

static int proto_40h_led_map(monome_t *monome, uint_t x_off, uint_t y_off,
                             const uint8_t *data) {
	uint8_t buf[8];
	int ret = 0;
	uint_t i;

	memcpy(buf, data, 8);
	ROTSPEC(monome).map_cb(monome, buf);

	for( i = 0; i < 8; i++ )
		ret += proto_40h_led_col_row(monome, PROTO_40h_LED_ROW, i, &buf[i]);

	return ret;
}

static monome_led_functions_t proto_40h_led_functions = {
	.set = proto_40h_led_set,
	.all = proto_40h_led_all,
	.map = proto_40h_led_map,
	.row = proto_40h_led_row,
	.col = proto_40h_led_col,
	.intensity = proto_40h_intensity
};

/**
 * tilt functions
 *
 * see http://post.monome.org/comments.php?DiscussionID=773#Item_5
 */

static int proto_40h_tilt_enable(monome_t *monome, uint_t sensor) {
	uint8_t buf[2] = {PROTO_40h_ADC_ENABLE, (sensor << 4) + 1};
	return monome_write(monome, buf, sizeof(buf));
}

static int proto_40h_tilt_disable(monome_t *monome, uint_t sensor) {
	uint8_t buf[2] = {PROTO_40h_ADC_ENABLE, (sensor << 4) + 0};
	return monome_write(monome, buf, sizeof(buf));
}

static monome_tilt_functions_t proto_40h_tilt_functions = {
	.enable = proto_40h_tilt_enable,
	.disable = proto_40h_tilt_disable
};

/**
 * module interface
 */

static int proto_40h_next_event(monome_t *monome, monome_event_t *e) {
	uint8_t buf[2] = {0, 0};
	ssize_t read;

	read = monome_platform_read(monome, buf, sizeof(buf));
	if (read < sizeof(buf))
		return read;

	switch( buf[0] ) {
	case PROTO_40h_BUTTON_DOWN:
	case PROTO_40h_BUTTON_UP:
		e->event_type = (buf[0] == PROTO_40h_BUTTON_DOWN) ? MONOME_BUTTON_DOWN : MONOME_BUTTON_UP;
		e->grid.x = buf[1] >> 4;
		e->grid.y = buf[1] & 0xF;

		UNROTATE_COORDS(monome, e->grid.x, e->grid.y);
		return 1;

	case PROTO_40h_AUX_1:
	case PROTO_40h_AUX_1 + 1:
	case PROTO_40h_AUX_1 + 2:
	case PROTO_40h_AUX_1 + 3:
		MONOME_40H_T(monome)->tilt.x = (((buf[0] & 0x3) << 8) | buf[1]) / 4;
		goto tilt_common; /* shut up okay */

	case PROTO_40h_AUX_2:
	case PROTO_40h_AUX_2 + 1:
	case PROTO_40h_AUX_2 + 2:
	case PROTO_40h_AUX_2 + 3:
		MONOME_40H_T(monome)->tilt.y = (((buf[0] & 0x3) << 8) | buf[1]) / 4;

tilt_common: /* I SAID SHUT UP */
		e->event_type = MONOME_TILT;
		e->tilt.sensor = 0;
		e->tilt.x = MONOME_40H_T(monome)->tilt.x;
		e->tilt.y = MONOME_40H_T(monome)->tilt.y;
		e->tilt.z = 0;
		return 1;
	}

	return 0;
}

static int proto_40h_open(monome_t *monome, const char *dev,
						  const char *serial, const monome_devmap_t *m,
						  va_list args) {
	monome->rows = m->dimensions.rows;
	monome->cols = m->dimensions.cols;
	monome->serial = serial;
	monome->friendly = m->friendly;

	return monome_platform_open(monome, m, dev);
}

static int proto_40h_close(monome_t *monome) {
	return monome_platform_close(monome);
}

static void proto_40h_free(monome_t *monome) {
	monome_40h_t *m40h = (monome_40h_t *) monome;
	m_free(m40h);
}

#if defined(EMBED_PROTOS)
monome_t *monome_protocol_40h_new(void) {
#else
monome_t *monome_protocol_new(void) {
#endif
	monome_t *monome = m_calloc(1, sizeof(monome_40h_t));

	if( !monome )
		return NULL;

	monome->open = proto_40h_open;
	monome->close = proto_40h_close;
	monome->free = proto_40h_free;

	monome->next_event = proto_40h_next_event;

	monome->led = &proto_40h_led_functions;
	monome->led_level = NULL;
	monome->led_ring = NULL;
	monome->tilt = &proto_40h_tilt_functions;

	MONOME_40H_T(monome)->tilt.x = 0;
	MONOME_40H_T(monome)->tilt.y = 0;

	return monome;
}
