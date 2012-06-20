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

#define PACKED __attribute__((__packed__))
#define MONOME_T(ptr) ((monome_t *) ptr)
#define MEXT_T(ptr) ((mext_t *) ptr)

/* protocol constants */

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
	/* outgoing */
	CMD_SYSTEM_QUERY       = 0x0,
	CMD_SYSTEM_GET_ID      = 0x1,
	CMD_SYSTEM_SET_ID      = 0x2,
	CMD_SYSTEM_GET_OFFSETS = 0x3,
	CMD_SYSTEM_SET_OFFSET  = 0x4,
	CMD_SYSTEM_GET_GRIDSZ  = 0x5,
	CMD_SYSTEM_SET_GRIDSZ  = 0x6,
	CMD_SYSTEM_GET_ADDR    = 0x7,
	CMD_SYSTEM_SET_ADDR    = 0x8,
	CMD_SYSTEM_GET_VERSION = 0xF,

	/* incoming */
	CMD_SYSTEM_QUERY_RESPONSE = 0x0,
	CMD_SYSTEM_ID             = 0x1,
	CMD_SYSTEM_GRID_OFFSET    = 0x2,
	CMD_SYSTEM_GRIDSZ         = 0x3,
	CMD_SYSTEM_ADDR           = 0x4,
	CMD_SYSTEM_VERSION        = 0xF,

	/* outgoing */
	CMD_LED_OFF          = 0x0,
	CMD_LED_ON           = 0x1,
	CMD_LED_ALL_OFF      = 0x2,
	CMD_LED_ALL_ON       = 0x3,
	CMD_LED_MAP          = 0x4,
	CMD_LED_ROW          = 0x5,
	CMD_LED_COLUMN       = 0x6,
	CMD_LED_INTENSITY    = 0x7,
	CMD_LED_LEVEL_SET    = 0x8,
	CMD_LED_LEVEL_ALL    = 0x9,
	CMD_LED_LEVEL_MAP    = 0xA,
	CMD_LED_LEVEL_ROW    = 0xB,
	CMD_LED_LEVEL_COLUMN = 0xC,

	/* incoming */
	CMD_KEY_UP   = 0x0,
	CMD_KEY_DOWN = 0x1,

	/* outgoing */
	CMD_LED_RING_SET   = 0x0,
	CMD_LED_RING_ALL   = 0x1,
	CMD_LED_RING_MAP   = 0x2,
	CMD_LED_RING_RANGE = 0x3,

	/* incoming */
	CMD_ENCODER_DELTA       = 0x0,
	CMD_ENCODER_SWITCH_UP   = 0x1,
	CMD_ENCODER_SWITCH_DOWN = 0x2,

	/* outgoing */
	CMD_TILT_STATE_REQ      = 0x0,
	CMD_TILT_ENABLE         = 0x1,
	CMD_TILT_DISABLE        = 0x2,

	/* incoming */
	CMD_TILT_STATES         = 0x0,
	CMD_TILT                = 0x1
} mext_cmd_t;

/* message lengths exclude one-byte header */
static size_t outgoing_payload_lengths[16][16] = {
	[0 ... 15][0 ... 15] = 0,

	[SS_SYSTEM] = {
		[CMD_SYSTEM_QUERY]       = 0,
		[CMD_SYSTEM_GET_ID]      = 0,
		[CMD_SYSTEM_SET_ID]      = 32,
		[CMD_SYSTEM_GET_OFFSETS] = 0,
		[CMD_SYSTEM_SET_OFFSET]  = 3,
		[CMD_SYSTEM_GET_GRIDSZ]  = 0,
		[CMD_SYSTEM_SET_GRIDSZ]  = 2,
		[CMD_SYSTEM_GET_ADDR]    = 0,
		[CMD_SYSTEM_SET_ADDR]    = 2,
		[CMD_SYSTEM_GET_VERSION] = 0,
	},

	[SS_LED_GRID] = {
		[CMD_LED_ON]           = 2,
		[CMD_LED_OFF]          = 2,
		[CMD_LED_ALL_ON]       = 0,
		[CMD_LED_ALL_OFF]      = 0,
		[CMD_LED_MAP]          = 10,
		[CMD_LED_ROW]          = 3,
		[CMD_LED_COLUMN]       = 3,
		[CMD_LED_INTENSITY]    = 1,
		[CMD_LED_LEVEL_SET]    = 3,
		[CMD_LED_LEVEL_ALL]    = 1,
		[CMD_LED_LEVEL_MAP]    = 34,
		[CMD_LED_LEVEL_ROW]    = 6,
		[CMD_LED_LEVEL_COLUMN] = 6
	},

	[SS_LED_RING] = {
		[CMD_LED_RING_SET]   = 3,
		[CMD_LED_RING_ALL]   = 2,
		[CMD_LED_RING_MAP]   = 33,
		[CMD_LED_RING_RANGE] = 4
	},

	[SS_TILT] = {
		[CMD_TILT_STATE_REQ] = 0,
		[CMD_TILT_ENABLE]    = 1,
		[CMD_TILT_DISABLE]   = 1
	}
};

static size_t incoming_payload_lengths[16][16] = {
	[0 ... 15][0 ... 15] = 0,

	[SS_SYSTEM] = {
		[CMD_SYSTEM_QUERY_RESPONSE] = 2,
		[CMD_SYSTEM_ID]             = 32,
		[CMD_SYSTEM_GRID_OFFSET]    = 3,
		[CMD_SYSTEM_GRIDSZ]         = 2,
		[CMD_SYSTEM_ADDR]           = 2,
		[CMD_SYSTEM_VERSION]        = 8
	},

	[SS_KEY_GRID] = {
		[CMD_KEY_DOWN] = 2,
		[CMD_KEY_UP]   = 2
	},

	[SS_ENCODER] = {
		[CMD_ENCODER_DELTA]       = 2,
		[CMD_ENCODER_SWITCH_UP]   = 1,
		[CMD_ENCODER_SWITCH_DOWN] = 1
	},

	[SS_TILT] = {
		[CMD_TILT_STATES] = 1,
		[CMD_TILT]        = 7
	}
};

/* types */

typedef struct mext mext_t;
typedef struct mext_msg mext_msg_t;
typedef struct mext_point mext_point_t;

/* a mext_handler_t should return 1 to propagate the event up to libmonome,
   or should return 0 if the event should not propagate/bubble. */

typedef int (*mext_handler_t)(mext_t *, mext_msg_t *, monome_event_t *);

struct mext {
	monome_t monome;

	char id[33];
};

struct mext_point {
	uint8_t x;
	uint8_t y;
} PACKED;

struct mext_msg {
	mext_subsystem_t addr;
	mext_cmd_t cmd;

	uint8_t header;

	union {
		/**
		  * system
		  */

		uint8_t id[32];

		mext_point_t gridsz;

		struct {
			uint8_t subsystem;
			uint8_t count;
		} PACKED query;

		/**
		 * led grid
		 */

		mext_point_t led;

		struct {
			mext_point_t offset;
			uint8_t data[8];
		} PACKED map;

		struct {
			mext_point_t offset;
			uint8_t data;
		} PACKED row_col;

		uint8_t intensity;

		struct {
			mext_point_t led;
			uint8_t level;
		} PACKED level_set;

		uint8_t level_all;

		struct {
			mext_point_t offset;
			uint8_t levels[64];
		} PACKED level_map;

		struct {
			mext_point_t offset;
			uint8_t levels[8];
		} PACKED level_row_col;

		/**
		 * key grid
		 */

		mext_point_t key;

		/**
		 * led ring
		 */

		struct {
			uint8_t ring;
			uint8_t led;
			uint8_t level;
		} PACKED led_ring_set;

		struct {
			uint8_t ring;
			uint8_t level;
		} PACKED led_ring_all;

		struct {
			uint8_t ring;
			uint8_t levels[64];
		} PACKED led_ring_map;

		struct {
			uint8_t ring;
			uint8_t start;
			uint8_t end;
			uint8_t level;
		} PACKED led_ring_range;

		/**
		 * encoder
		 */

		struct {
			uint8_t number;
			int8_t delta;
		} PACKED encoder;

		/**
		 * tilt
		 */

		struct {
			uint8_t number;
		} PACKED tilt_sys;

		struct {
			uint8_t number;
			int16_t x;
			int16_t y;
			int16_t z;
		} PACKED tilt;
	} PACKED payload;
} PACKED;
