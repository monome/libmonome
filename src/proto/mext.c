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

#define SELF_FROM(monome) mext_t *self = (mext_t *) monome

/* message length excludes the one-byte header */
static int msg_lengths[] = {
	[CMD_LED_ON]        = 1,
	[CMD_LED_OFF]       = 1,
	[CMD_LED_ALL_ON]    = 0,
	[CMD_LED_ALL_OFF]   = 0,
	[CMD_LED_FRAME]     = 10,
	[CMD_LED_ROW]       = 3,
	[CMD_LED_COLUMN]    = 3,
	[CMD_LED_INTENSITY] = 1
};

/* private */

static int mext_write_msg(monome_t *monome, mext_msg_t *msg) {
	monome_platform_write(monome, (uint8_t *) msg, msg_lengths[msg->header.cmd]);

	return 0;
}

/* public */

static int mext_led(monome_t *monome, uint_t x, uint_t y, uint_t on) {
	mext_msg_t msg = {
		.header = {
			.addr = SS_LED_GRID,
			.cmd  = !on
		}
	};

	ROTATE_COORDS(monome, x, y);

	msg.cmd.led.x = x;
	msg.cmd.led.y = y;

	return mext_write_msg(monome, &msg);
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

	monome->next_event = NULL;

	monome->clear      = NULL;
	monome->intensity  = NULL;
	monome->mode       = NULL;
	
	monome->led        = mext_led;
	monome->led_col    = NULL;
	monome->led_row    = NULL;
	monome->led_frame  = NULL;

	return NULL;
}
