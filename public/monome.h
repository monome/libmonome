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

#ifndef _MONOME_H
#define _MONOME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <termios.h>

#include <sys/types.h>

typedef enum {
	MONOME_BUTTON_UP     = 0x00,
	MONOME_BUTTON_DOWN   = 0x01,
	MONOME_AUX_INPUT     = 0x02
} monome_event_type_t;

/* clearing statuses (argument to monome_clear) */

typedef enum {
	MONOME_CLEAR_OFF     = 0x00,
	MONOME_CLEAR_ON      = 0x01
} monome_clear_status_t;

/* modes (argument to monome_mode) */

typedef enum {
	MONOME_MODE_NORMAL   = 0x00,
	MONOME_MODE_TEST     = 0x01,
	MONOME_MODE_SHUTDOWN = 0x02
} monome_mode_t;

/* cable orientation */

typedef enum {
	MONOME_CABLE_LEFT    = 0,
	MONOME_CABLE_BOTTOM  = 1,
	MONOME_CABLE_RIGHT   = 2,
	MONOME_CABLE_TOP     = 3
} monome_cable_t;
	
typedef struct monome_event monome_event_t;
typedef struct monome monome_t; /* opaque data type */

typedef void (*monome_event_callback_t)
	(const monome_event_t *event, void *data);

struct monome_event {
	monome_t *monome;
	monome_event_type_t event_type;

	union {
		uint x;
		uint address;
	};

	union {
		uint y;
		uint data;
	};
};

monome_t *monome_open(const char *monome_device, ...);
void monome_close(monome_t *monome);

void monome_set_orientation(monome_t *monome, monome_cable_t cable);
monome_cable_t monome_get_orientation(monome_t *monome);

const char *monome_get_serial(monome_t *monome);
const char *monome_get_devpath(monome_t *monome);
int monome_get_rows(monome_t *monome);
int monome_get_cols(monome_t *monome);

int monome_register_handler(monome_t *monome, monome_event_type_t event_type,
							monome_event_callback_t, void *user_data);
int monome_unregister_handler(monome_t *monome,
							  monome_event_type_t event_type);
int monome_event_next(monome_t *monome, monome_event_t *event_buf);
int monome_event_handle_next(monome_t *monome);
void monome_event_loop(monome_t *monome);
int monome_get_fd(monome_t *monome);

int monome_clear(monome_t *monome, monome_clear_status_t status);
int monome_intensity(monome_t *monome, uint brightness);
int monome_mode(monome_t *monome, monome_mode_t mode);

int monome_led(monome_t *monome, uint x, uint y, uint on);
int monome_led_on(monome_t *monome, uint x, uint y);
int monome_led_off(monome_t *monome, uint x, uint y);
int monome_led_col(monome_t *monome, uint col, size_t count,
				   const uint8_t *col_data);
int monome_led_row(monome_t *monome, uint row, size_t count,
				   const uint8_t *row_data);
int monome_led_frame(monome_t *monome, uint quadrant,
					 const uint8_t *frame_data);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* defined _MONOME_H */
