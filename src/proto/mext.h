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

#include <stdint.h>

#include <monome.h>
#include "internal.h"

#define PACKED __attribute__((packed))

typedef monome_t mext_t;

typedef enum {
	SS_SYSTEM      = 0,
	SS_LED_GRID    = 1,
	SS_KEY_GRID    = 2,
	SS_DIGITAL_OUT = 3,
	SS_DIGITAL_IN  = 4,
	SS_ENCODER     = 5,
	SS_ANALOG_IN   = 6,
	SS_ANALOG_OUT  = 7,
	SS_TILT        = 8,
	SS_LED_RING    = 9
} mext_subsystem_t;

typedef enum {
	CMD_LED_ON        = 0,
	CMD_LED_OFF       = 1,
	CMD_LED_ALL_ON    = 2,
	CMD_LED_ALL_OFF   = 3,
	CMD_LED_FRAME     = 4,
	CMD_LED_ROW       = 5,
	CMD_LED_COLUMN    = 6,
	CMD_LED_INTENSITY = 7
} mext_cmd_t;

typedef struct mext_msg mext_msg_t;

struct PACKED mext_msg {
	struct PACKED {
		__extension__ mext_subsystem_t addr:4;
		__extension__ mext_cmd_t cmd:4;
	} header;

	union PACKED {
		struct PACKED {
			uint8_t x;
			uint8_t y;
		} led;

		struct PACKED {
			struct PACKED {
				uint8_t x;
				uint8_t y;
			} offset;

			uint8_t data[8];
		} frame;
	} cmd;
};
