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

#include "mext.h"

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(*x))
#define SELF_FROM(monome) mext_t *self = MEXT_T(monome)

/**
 * protocol internal
 */

static ssize_t mext_write_msg(monome_t *monome, mext_msg_t *msg) {
	size_t payload_length;

	payload_length = outgoing_payload_lengths[msg->addr][msg->cmd];
	msg->header = ((msg->addr & 0xF ) << 4) | (msg->cmd & 0xF);

	return monome_platform_write(monome, &msg->header, 1 + payload_length);
}

static ssize_t mext_read_msg(monome_t *monome, mext_msg_t *msg) {
	size_t payload_length;
	ssize_t read;

	if ((read = monome_platform_read(monome, &msg->header, 1)) <= 0)
		return read;

	msg->addr = msg->header >> 4;
	msg->cmd  = msg->header & 0xF;

	payload_length = incoming_payload_lengths[msg->addr][msg->cmd];

	if( !payload_length )
		return 1;

	if( monome_platform_read(monome, (uint8_t *) &msg->payload, payload_length)
		!= payload_length )
		return -1;

	return 1 + payload_length;
}

static ssize_t mext_simple_cmd(monome_t *monome, mext_cmd_t cmd) {
	mext_msg_t msg = {
		.addr = SS_SYSTEM,
		.cmd  = cmd
	};

	return mext_write_msg(monome, &msg);
}

static ssize_t mext_led_row_col(monome_t *monome, mext_cmd_t cmd, uint_t x,
                                uint_t y, uint8_t data) {
	mext_msg_t msg = {
		.addr = SS_LED_GRID,
		.cmd  = cmd
	};

	if( ROTSPEC(monome).flags & ROW_COL_SWAP )
		msg.cmd = !(cmd - CMD_LED_ROW) + CMD_LED_ROW;

	ROTATE_COORDS(monome, x, y);

	msg.payload.row_col.offset.x = x;
	msg.payload.row_col.offset.y = y;
	msg.payload.row_col.data     = data;

	return mext_write_msg(monome, &msg);
}

static void revcopy(uint8_t *dst, const uint8_t *src) {
	int i = 8;

	while( i-- )
		dst[7 - i] = src[i];
}

static void pack_nybbles(uint8_t *data, size_t nbyte) {
	uint_t i;

	for( i = 0; i < nbyte; i++ )
		data[i] =
			(data[i * 2] << 4) |
			(data[(i * 2) + 1] & 0x0F);
}

static ssize_t mext_led_level_row_col(monome_t *monome, mext_cmd_t cmd, int rev,
                                      uint_t x, uint_t y, const uint8_t *data) {
	mext_msg_t msg = {
		.addr = SS_LED_GRID,
		.cmd  = cmd
	};

	if( ROTSPEC(monome).flags & ROW_COL_SWAP )
		msg.cmd = !(cmd - CMD_LED_LEVEL_ROW) + CMD_LED_LEVEL_ROW;

	ROTATE_COORDS(monome, x, y);

	msg.payload.row_col.offset.x = x;
	msg.payload.row_col.offset.y = y;

	if( rev )
		revcopy(msg.payload.level_row_col.levels, data);
	else
		memcpy(msg.payload.level_row_col.levels, data, 8);

	pack_nybbles(msg.payload.level_row_col.levels, 4);
	return mext_write_msg(monome, &msg);
}

/**
 * led functions
 */

static int mext_led_set(monome_t *monome, uint_t x, uint_t y, uint_t on) {
	mext_msg_t msg = {
		.addr = SS_LED_GRID,
		.cmd  = on ? CMD_LED_ON : CMD_LED_OFF
	};

	ROTATE_COORDS(monome, x, y);

	msg.payload.led.x = x;
	msg.payload.led.y = y;

	return mext_write_msg(monome, &msg);
}

static int mext_led_all(monome_t *monome, uint_t status) {
	mext_msg_t msg = {
		.addr = SS_LED_GRID,
		.cmd = (status) ? CMD_LED_ALL_ON : CMD_LED_ALL_OFF
	};


	return mext_write_msg(monome, &msg);
}

static int mext_led_map(monome_t *monome, uint_t x_off, uint_t y_off,
                        const uint8_t *data) {
	mext_msg_t msg = {
		.addr = SS_LED_GRID,
		.cmd  = CMD_LED_MAP
	};

	memcpy(msg.payload.map.data, data, 8);
	ROTSPEC(monome).map_cb(monome, msg.payload.map.data);

	ROTATE_COORDS(monome, x_off, y_off);
	msg.payload.map.offset.x = x_off;
	msg.payload.map.offset.y = y_off;

	return mext_write_msg(monome, &msg);
}

static int mext_led_row(monome_t *monome, uint_t x_off, uint_t y,
                        size_t count, const uint8_t *data) {
	if( ROTSPEC(monome).flags & ROW_REVBITS ) {
		for( ; count--; x_off += 8, data++ )
			mext_led_row_col(
				monome, CMD_LED_ROW, x_off, y, REVERSE_BYTE(*data));
	} else {
		for( ; count--; x_off += 8, data++ )
			mext_led_row_col(
				monome, CMD_LED_ROW, x_off, y, *data);
	}

	return 1;
}

static int mext_led_col(monome_t *monome, uint_t x, uint_t y_off,
                        size_t count, const uint8_t *data) {
	if( ROTSPEC(monome).flags & COL_REVBITS ) {
		for( ; count--; y_off += 8, data++ )
			mext_led_row_col(
				monome, CMD_LED_COLUMN, x, y_off, REVERSE_BYTE(*data));
	} else {
		for( ; count--; y_off += 8, data++ )
			mext_led_row_col(
				monome, CMD_LED_COLUMN, x, y_off, *data);
	}

	return 1;
}

static int mext_led_intensity(monome_t *monome, uint_t intensity) {
	mext_msg_t msg = {
		.addr = SS_LED_GRID,
		.cmd  = CMD_LED_INTENSITY
	};

	msg.payload.intensity = intensity & 0xF;
	return mext_write_msg(monome, &msg);
}

static monome_led_functions_t mext_led_functions = {
	.set = mext_led_set,
	.all = mext_led_all,
	.col = mext_led_col,
	.row = mext_led_row,
	.map = mext_led_map,
	.intensity = mext_led_intensity
};

/**
 * led level functions
 */

static int mext_led_level_set(monome_t *monome, uint_t x, uint_t y,
                              uint_t level) {
	mext_msg_t msg = {
		.addr = SS_LED_GRID,
		.cmd  = CMD_LED_LEVEL_SET,

		.payload = {
			.level_set = {
				.level = level
			}
		}
	};

	ROTATE_COORDS(monome, x, y);

	msg.payload.level_set.led.x = x;
	msg.payload.level_set.led.y = y;

	return mext_write_msg(monome, &msg);
}

static int mext_led_level_all(monome_t *monome, uint_t level) {
	mext_msg_t msg = {
		.addr = SS_LED_GRID,
		.cmd  = CMD_LED_LEVEL_ALL,

		.payload = {
			.level_all = level
		}
	};

	return mext_write_msg(monome, &msg);
}

static int mext_led_level_map(monome_t *monome, uint_t x_off, uint_t y_off,
                              const uint8_t *data) {
	mext_msg_t msg = {
		.addr = SS_LED_GRID,
		.cmd  = CMD_LED_LEVEL_MAP
	};

	ROTATE_COORDS(monome, x_off, y_off);
	ROTSPEC(monome).level_map_cb(monome, msg.payload.level_map.levels, data);
	pack_nybbles(msg.payload.level_map.levels, 32);

	msg.payload.level_map.offset.x = x_off;
	msg.payload.level_map.offset.y = y_off;

	return mext_write_msg(monome, &msg);
}

static int mext_led_level_row(monome_t *monome, uint_t x_off, uint_t row,
                              size_t count, const uint8_t *data) {
	for( count >>= 3; count--; x_off += 8, data += 8 )
		mext_led_level_row_col(
			monome, CMD_LED_LEVEL_ROW, ROTSPEC(monome).flags & ROW_REVBITS,
			x_off, row, data);

	return 1;
}

static int mext_led_level_col(monome_t *monome, uint_t col, uint_t y_off,
                              size_t count, const uint8_t *data) {
	for( count >>= 3; count--; y_off += 8, data += 8 )
		mext_led_level_row_col(
			monome, CMD_LED_LEVEL_COLUMN, ROTSPEC(monome).flags & COL_REVBITS,
			col, y_off, data);

	return 1;
}

static monome_led_level_functions_t mext_led_level_functions = {
	.set = mext_led_level_set,
	.all = mext_led_level_all,
	.map = mext_led_level_map,
	.row = mext_led_level_row,
	.col = mext_led_level_col
};

/**
 * led ring functions
 */

static int mext_led_ring_set(monome_t *monome, uint_t ring, uint_t led,
                             uint_t level) {
	mext_msg_t msg = {
		.addr = SS_LED_RING,
		.cmd  = CMD_LED_RING_SET,

		.payload = {
			.led_ring_set = {
				.ring  = ring,
				.led   = led,
				.level = level
			}
		}
	};

	return mext_write_msg(monome, &msg);
}

static int mext_led_ring_all(monome_t *monome, uint_t ring, uint_t level) {
	mext_msg_t msg = {
		.addr = SS_LED_RING,
		.cmd  = CMD_LED_RING_ALL,

		.payload = {
			.led_ring_all = {
				.ring  = ring,
				.level = level
			}
		}
	};

	return mext_write_msg(monome, &msg);
}

static int mext_led_ring_map(monome_t *monome, uint_t ring,
                             const uint8_t *levels) {
	mext_msg_t msg = {
		.addr = SS_LED_RING,
		.cmd  = CMD_LED_RING_MAP,

		.payload = {
			.led_ring_map = {
				.ring  = ring
			}
		}
	};

	memcpy(msg.payload.led_ring_map.levels, levels, 64);
	pack_nybbles(msg.payload.led_ring_map.levels, 32);

	return mext_write_msg(monome, &msg);
}

static int mext_led_ring_range(monome_t *monome, uint_t ring, uint_t start,
                               uint_t end, uint_t level) {
	mext_msg_t msg = {
		.addr = SS_LED_RING,
		.cmd  = CMD_LED_RING_RANGE,

		.payload = {
			.led_ring_range = {
				.ring  = ring,
				.start = start,
				.end   = end,
				.level = level
			}
		}
	};

	return mext_write_msg(monome, &msg);
}

static monome_led_ring_functions_t mext_led_ring_functions = {
	.set   = mext_led_ring_set,
	.all   = mext_led_ring_all,
	.map   = mext_led_ring_map,
	.range = mext_led_ring_range
};

/**
 * tilt functions
 */

static int mext_tilt_enable(monome_t *monome, uint_t sensor) {
	mext_msg_t msg = {
		.addr = SS_TILT,
		.cmd  = CMD_TILT_ENABLE,

		.payload = {
			.tilt_sys = {
				.number = sensor
			}
		}
	};

	return mext_write_msg(monome, &msg);
}

static int mext_tilt_disable(monome_t *monome, uint_t sensor) {
	mext_msg_t msg = {
		.addr = SS_TILT,
		.cmd  = CMD_TILT_DISABLE,

		.payload = {
			.tilt_sys = {
				.number = sensor
			}
		}
	};

	return mext_write_msg(monome, &msg);
}

static monome_tilt_functions_t mext_tilt_functions = {
	.enable = mext_tilt_enable,
	.disable = mext_tilt_disable
};

/**
 * event handlers
 */

static int mext_handler_noop(mext_t *self, mext_msg_t *msg, monome_event_t *e) {
	return 0;
}

static int mext_handler_system(mext_t *self, mext_msg_t *msg, monome_event_t *e) {
	switch( msg->cmd ) {
	case CMD_SYSTEM_QUERY_RESPONSE:
		break;

	case CMD_SYSTEM_ID:
		strncpy(self->id, (char *) msg->payload.id, 32);
		self->id[32] = '\0'; /* just in case */

		MONOME_T(self)->friendly = self->id;
		break;

	case CMD_SYSTEM_GRID_OFFSET:
		break;

	case CMD_SYSTEM_GRIDSZ:
		MONOME_T(self)->cols = msg->payload.gridsz.x;
		MONOME_T(self)->rows = msg->payload.gridsz.y;
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

static int mext_handler_key_grid(mext_t *self, mext_msg_t *msg, monome_event_t *e) {
	e->event_type = ( msg->cmd == CMD_KEY_DOWN ) ? MONOME_BUTTON_DOWN : MONOME_BUTTON_UP;
	e->grid.x = msg->payload.key.x;
	e->grid.y = msg->payload.key.y;
	UNROTATE_COORDS(MONOME_T(self), e->grid.x, e->grid.y);

	return 1;
}

static int mext_handler_encoder(mext_t *self, mext_msg_t *msg, monome_event_t *e) {
	switch( msg->cmd ) {
	case CMD_ENCODER_DELTA:
		e->event_type = MONOME_ENCODER_DELTA;
		e->encoder.number = msg->payload.encoder.number;
		e->encoder.delta = msg->payload.encoder.delta;
		return 1;

	case CMD_ENCODER_SWITCH_DOWN:
		e->event_type = MONOME_ENCODER_KEY_DOWN;
		e->encoder.number = msg->payload.encoder.number;
		e->encoder.delta = 0;
		return 1;

	case CMD_ENCODER_SWITCH_UP:
		e->event_type = MONOME_ENCODER_KEY_UP;
		e->encoder.number = msg->payload.encoder.number;
		e->encoder.delta = 0;
		return 1;

	default:
		break;
	}

	return 0;
}

static int mext_handler_tilt(mext_t *self, mext_msg_t *msg, monome_event_t *e) {
	switch( msg->cmd ) {
	case CMD_TILT_STATES:
		break;

	case CMD_TILT:
		e->event_type = MONOME_TILT;
		e->tilt.sensor = msg->payload.tilt.number;
		e->tilt.x = msg->payload.tilt.x;
		e->tilt.y = msg->payload.tilt.y;
		e->tilt.z = msg->payload.tilt.z;

		return 1;

	default:
		break;
	}

	return 0;
}

static mext_handler_t subsystem_event_handlers[16] = {
	[0 ... 15] = &mext_handler_noop,

	[SS_SYSTEM]   = mext_handler_system,
	[SS_KEY_GRID] = mext_handler_key_grid,
	[SS_ENCODER]  = mext_handler_encoder,
	[SS_TILT]     = mext_handler_tilt
};

/**
 * device control functions
 */

static int mext_next_event(monome_t *monome, monome_event_t *e) {
	SELF_FROM(monome);
	mext_msg_t msg = {0, 0};
	ssize_t status;

	while ((status = mext_read_msg(monome, &msg)) > 0) {
		if (msg.addr == SS_SYSTEM) {
			subsystem_event_handlers[0](self, &msg, e);
			continue;
		}

		if (subsystem_event_handlers[msg.addr](self, &msg, e))
			return 1;
	}

	return status;
}

static int mext_open(monome_t *monome, const char *dev, const char *serial,
                     const monome_devmap_t *m, va_list args) {
	int i;
	monome_event_t e;
	mext_cmd_t startup_cmds[] = {
		CMD_SYSTEM_QUERY,
		CMD_SYSTEM_GET_ID,
		CMD_SYSTEM_GET_GRIDSZ
	};

	if( monome_platform_open(monome, m, dev) )
		return 1;

	monome->serial = serial;
	monome->friendly = m->friendly;

	mext_simple_cmd(monome, CMD_SYSTEM_QUERY);
	mext_simple_cmd(monome, CMD_SYSTEM_GET_ID);
	mext_simple_cmd(monome, CMD_SYSTEM_GET_GRIDSZ);

	for( i = 0; i < ARRAY_LENGTH(startup_cmds); i++ ) {
		mext_simple_cmd(monome, startup_cmds[i]);
		monome_platform_wait_for_input(monome, 250);
		mext_next_event(monome, &e);
	}

	return 0;
}

static int mext_close(monome_t *monome) {
	return monome_platform_close(monome);
}

static void mext_free(monome_t *monome) {
	SELF_FROM(monome);

	m_free(self);
}

#if defined(EMBED_PROTOS)
monome_t *monome_protocol_mext_new(void) {
#else
monome_t *monome_protocol_new(void) {
#endif
	mext_t *self = m_calloc(1, sizeof(mext_t));
	monome_t *monome = MONOME_T(self);

	if( !monome )
		return NULL;

	monome->open  = mext_open;
	monome->close = mext_close;
	monome->free  = mext_free;

	monome->next_event = mext_next_event;

	monome->led = &mext_led_functions;
	monome->led_level = &mext_led_level_functions;
	monome->led_ring = &mext_led_ring_functions;
	monome->tilt = &mext_tilt_functions;

	return monome;
}
