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

#define MONOME_40H_T(x) ((monome_40h_t *) x)

typedef enum {
	/* input (from device) */

	PROTO_40h_BUTTON_DOWN         = 0x01,
	PROTO_40h_BUTTON_UP           = 0x00,
	PROTO_40h_AUX_1               = 0x10,
	PROTO_40h_AUX_2               = 0x14,

	/* output (to device) */

	PROTO_40h_LED_ON              = 0x21,
	PROTO_40h_LED_OFF             = 0x20,
	PROTO_40h_INTENSITY           = 0x30,
	PROTO_40h_LED_TEST            = 0x40,
	PROTO_40h_ADC_ENABLE          = 0x50,
	PROTO_40h_SHUTDOWN            = 0x60,
	PROTO_40h_LED_ROW             = 0x70,
	PROTO_40h_LED_COL             = 0x80,
} proto_40h_message_t;


typedef struct monome_40h monome_40h_t;

struct monome_40h {
	monome_t parent;
	monome_mode_t mode;
	
	struct {
		int x;
		int y;
	} tilt;
};

