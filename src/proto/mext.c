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

	payload_length = outgoing_payload_lengths[msg->hdr.addr][msg->hdr.cmd];
	monome_platform_write(monome, (uint8_t *) msg, 1 + payload_length);

	return 0;
}

static int mext_read_msg(monome_t *monome, mext_msg_t *msg) {
	size_t payload_length;

	monome_platform_read(monome, (uint8_t *) msg, 1);
	payload_length = incoming_payload_lengths[msg->hdr.addr][msg->hdr.cmd];

	if( !payload_length )
		return 0;

	if( monome_platform_read(monome, (uint8_t *) &msg->payload, payload_length)
		!= payload_length )
		return 1;

	return 0;
}

/**
 * led functions
 */

static int mext_led(monome_t *monome, uint_t x, uint_t y, uint_t on) {
	mext_msg_t msg = {
		.hdr = {
			.addr = SS_LED_GRID,
			.cmd  = !on
		}
	};

	ROTATE_COORDS(monome, x, y);

	msg.payload.led.x = x;
	msg.payload.led.y = y;

	return mext_write_msg(monome, &msg);
}

static int mext_led_frame(monome_t *monome, uint_t x_off, uint_t y_off,
                          const uint8_t *frame_data) {
	mext_msg_t msg = {
		.hdr = {
			.addr = SS_LED_GRID,
			.cmd  = CMD_LED_FRAME
		}
	};

#ifdef __LP64__
	*((uint64_t *) msg.payload.frame.data) = *((uint64_t *) frame_data);
#else
	*((uint32_t *) msg.payload.frame.data) = *((uint32_t *) frame_data);
	*((uint32_t *) msg.payload.frame.data + 4) = *(((uint32_t *) frame_data) + 1);
#endif

	/* XXX: rotation needs to take offsets instead of quadrant */
	/* ROTSPEC(monome).frame_cb(monome, &x_off, &y_off, msg.cmd.frame.data); */

	return mext_write_msg(monome, &msg);
}

/**
 * device control functions
 */

static int mext_next_event(monome_t *monome, monome_event_t *e) {
	mext_msg_t msg = {{0, 0}};

	if( mext_read_msg(monome, &msg) )
		return 0;

	return 0;
}

static int mext_open(monome_t *monome, const char *dev, const char *serial,
                     const monome_devmap_t *m, va_list args) {
	if( monome_platform_open(monome, dev) )
		return 1;

	monome->serial = serial;

	/* request grid size here, etc */

	return 0;
}

static void mext_free(monome_t *monome) {
	SELF_FROM(monome);

	free(self);
}

monome_t *monome_protocol_new() {
	mext_t *self = calloc(1, sizeof(mext_t));
	monome_t *monome = self;

	if( !monome )
		return NULL;

	monome->open       = mext_open;
	monome->close      = NULL;
	monome->free       = mext_free;

	monome->next_event = mext_next_event;

	monome->clear      = NULL;
	monome->intensity  = NULL;
	monome->mode       = NULL;
	
	monome->led        = mext_led;
	monome->led_col    = NULL;
	monome->led_row    = NULL;
	monome->led_frame  = mext_led_frame;

	return NULL;
}
