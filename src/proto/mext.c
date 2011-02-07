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

#include <monome.h>
#include "internal.h"
#include "platform.h"
#include "rotation.h"

#include "mext.h"

#define SELF_FROM(monome) mext_t *self = MEXT_T(monome)

/**
 * protocol internal
 */

static int mext_write_msg(monome_t *monome, mext_msg_t *msg) {
	size_t payload_length;

	payload_length = outgoing_payload_lengths[msg->addr][msg->cmd];

	msg->header = ((msg->addr & 0x7 ) << 4) | (msg->cmd & 0x7);
	monome_platform_write(monome, &msg->header, 1 + payload_length);

	return 0;
}

static int mext_read_msg(monome_t *monome, mext_msg_t *msg) {
	size_t payload_length;

	monome_platform_read(monome, &msg->header, 1);
	msg->addr = msg->header >> 4;
	msg->cmd  = msg->header & 0x7;

	payload_length = incoming_payload_lengths[msg->addr][msg->cmd];

	if( !payload_length )
		return 1;

	if( monome_platform_read(monome, (uint8_t *) &msg->payload, payload_length)
		!= payload_length )
		return 0;

	return 1 + payload_length;
}

static int mext_simple_cmd(monome_t *monome, mext_cmd_t cmd) {
	mext_msg_t msg = {
		.addr = SS_SYSTEM,
		.cmd  = cmd
	};

	return mext_write_msg(monome, &msg);
}

/**
 * led functions
 */

static int mext_mode_noop(monome_t *monome, monome_mode_t mode) {
	/* unimplemented */
	return 0;
}

static int mext_clear(monome_t *monome, monome_clear_status_t status) {
	mext_msg_t msg = {
		.addr = SS_LED_GRID
	};

	if( status == MONOME_CLEAR_OFF )
		msg.cmd = CMD_LED_ALL_OFF;
	else
		msg.cmd = CMD_LED_ALL_ON;

	return mext_write_msg(monome, &msg);
}

static int mext_intensity(monome_t *monome, uint_t intensity) {
	mext_msg_t msg = {
		.addr = SS_LED_GRID,
		.cmd  = CMD_LED_INTENSITY
	};

	msg.payload.intensity = intensity & 0xF;
	return mext_write_msg(monome, &msg);
}

static int mext_led(monome_t *monome, uint_t x, uint_t y, uint_t on) {
	mext_msg_t msg = {
		.addr = SS_LED_GRID,
		.cmd  = on ? CMD_LED_ON : CMD_LED_OFF
	};

	ROTATE_COORDS(monome, x, y);

	msg.payload.led.x = x;
	msg.payload.led.y = y;

	return mext_write_msg(monome, &msg);
}

static int mext_led_col(monome_t *monome, uint_t col, size_t count,
                        const uint8_t *data) {
	return 0;
}

static int mext_led_row(monome_t *monome, uint_t row, size_t count,
                        const uint8_t *data) {
	return 0;
}

static int mext_led_frame(monome_t *monome, uint_t x_off, uint_t y_off,
                          const uint8_t *frame_data) {
	mext_msg_t msg = {
		.addr = SS_LED_GRID,
		.cmd  = CMD_LED_FRAME
	};

#ifdef __LP64__
	*((uint64_t *) msg.payload.frame.data) = *((uint64_t *) frame_data);
#else
	*((uint32_t *) msg.payload.frame.data) = *((uint32_t *) frame_data);
	*((uint32_t *) msg.payload.frame.data + 4) = *(((uint32_t *) frame_data) + 1);
#endif

	ROTSPEC(monome).frame_cb(monome, &x_off, &y_off, msg.payload.frame.data);

	return mext_write_msg(monome, &msg);
}

/**
 * event handlers
 */

int mext_handler_noop(mext_t *self, mext_msg_t *msg, monome_event_t *e) {
	return 1;
}

int mext_handler_system(mext_t *self, mext_msg_t *msg, monome_event_t *e) {
	switch( msg->cmd ) {
	case CMD_SYSTEM_QUERY_RESPONSE:
		break;

	case CMD_SYSTEM_ID:
		break;

	case CMD_SYSTEM_GRID_OFFSET:
		break;

	case CMD_SYSTEM_GRIDSZ:
		MONOME_T(self)->rows = msg->payload.gridsz.x;
		MONOME_T(self)->cols = msg->payload.gridsz.y;
		break;

	case CMD_SYSTEM_ADDR:
		break;

	case CMD_SYSTEM_VERSION:
		break;

	default:
		break;
	}

	return 0;
}

int mext_handler_key_grid(mext_t *self, mext_msg_t *msg, monome_event_t *e) {
	e->event_type = ( msg->cmd == CMD_KEY_DOWN ) ? MONOME_BUTTON_DOWN : MONOME_BUTTON_UP;
	e->x = msg->payload.key.x;
	e->y = msg->payload.key.y;
	return 1;
}

static mext_handler_t subsystem_event_handlers[16] = {
	[0 ... 15] = mext_handler_noop,
	
	[SS_SYSTEM]   = mext_handler_system,
	[SS_KEY_GRID] = mext_handler_key_grid
};

/**
 * device control functions
 */

static int mext_next_event(monome_t *monome, monome_event_t *e) {
	SELF_FROM(monome);
	mext_msg_t msg = {0, 0};
	int ret;

	while( mext_read_msg(monome, &msg) ) {
		ret = subsystem_event_handlers[msg.addr](self, &msg, e);

		if( msg.addr != SS_SYSTEM )
			return ret;
	}

	return 0;
}

static int mext_open(monome_t *monome, const char *dev, const char *serial,
                     const monome_devmap_t *m, va_list args) {
	monome_event_t e;

	if( monome_platform_open(monome, dev) )
		return 1;

	monome->serial = serial;

	mext_simple_cmd(monome, CMD_SYSTEM_QUERY);
	mext_simple_cmd(monome, CMD_SYSTEM_GET_GRIDSZ);

	monome_platform_wait_for_input(monome, 150);

	while( mext_next_event(monome, &e) );

	return 0;
}

static int mext_close(monome_t *monome) {
	return monome_platform_close(monome);
}

static void mext_free(monome_t *monome) {
	SELF_FROM(monome);

	free(self);
}

monome_t *monome_protocol_new() {
	mext_t *self = calloc(1, sizeof(mext_t));
	monome_t *monome = MONOME_T(self);

	if( !monome )
		return NULL;

	monome->open       = mext_open;
	monome->close      = mext_close;
	monome->free       = mext_free;

	monome->next_event = mext_next_event;

	monome->clear      = mext_clear;
	monome->intensity  = mext_intensity;
	monome->mode       = mext_mode_noop;
	
	monome->led        = mext_led;
	monome->led_col    = mext_led_row;
	monome->led_row    = mext_led_col;
	monome->led_frame  = mext_led_frame;

	return monome;
}
