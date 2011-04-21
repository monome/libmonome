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

#include "monome.h"
#include "internal.h"

#define SERIES_T(x) ((series_t *) x)
typedef struct series series_t;

typedef enum {
	/* input (from device) */

	PROTO_SERIES_BUTTON_DOWN         = 0x00,
	PROTO_SERIES_BUTTON_UP           = 0x10,
	PROTO_SERIES_TILT                = 0xD0,
	PROTO_SERIES_AUX_INPUT           = 0xE0,

	/* output (to device) */

	PROTO_SERIES_LED_ON              = 0x20,
	PROTO_SERIES_LED_OFF             = 0x30,
	PROTO_SERIES_LED_ROW_8           = 0x40,
	PROTO_SERIES_LED_COL_8           = 0x50,
	PROTO_SERIES_LED_ROW_16          = 0x60,
	PROTO_SERIES_LED_COL_16          = 0x70,
	PROTO_SERIES_LED_FRAME           = 0x80,
	PROTO_SERIES_CLEAR               = 0x90,
	PROTO_SERIES_INTENSITY           = 0xA0,
	PROTO_SERIES_MODE                = 0xB0,
	PROTO_SERIES_AUX_PORT_ACTIVATE   = 0xC0,
	PROTO_SERIES_AUX_PORT_DEACTIVATE = 0xD0
} proto_series_message_t;

/* modes (argument to the PROTO_SERIES_MODE output command) */

typedef enum {
	PROTO_SERIES_MODE_NORMAL         = 0x00,
	PROTO_SERIES_MODE_TEST           = 0x01,
	PROTO_SERIES_MODE_SHUTDOWN       = 0x02
} proto_series_mode_t;

struct series {
	monome_t monome;

	struct {
		int x;
		int y;
	} tilt;
};
