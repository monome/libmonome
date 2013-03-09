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

#ifndef _MONOME_H
#define _MONOME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

#include <sys/types.h>

/* event types */

typedef enum {
	MONOME_BUTTON_UP        = 0x00,
	MONOME_BUTTON_DOWN      = 0x01,
	MONOME_ENCODER_DELTA    = 0x02,
	MONOME_ENCODER_KEY_UP   = 0x03,
	MONOME_ENCODER_KEY_DOWN = 0x04,
	MONOME_TILT             = 0x05,

	/* update this if you add event types */
	MONOME_EVENT_MAX        = 0x06
} monome_event_type_t;

/* grid rotation */

typedef enum {
	MONOME_ROTATE_0    = 0,
	MONOME_ROTATE_90   = 1,
	MONOME_ROTATE_180  = 2,
	MONOME_ROTATE_270  = 3
} monome_rotate_t;

typedef struct monome monome_t; /* opaque data type */
typedef struct monome_event monome_event_t;

typedef void (*monome_event_callback_t)
	(const monome_event_t *event, void *data);

struct monome_event {
	monome_t *monome;
	monome_event_type_t event_type;

	/* __extension__ for anonymous unions in gcc */
	__extension__ union {
		struct {
			unsigned int x;
			unsigned int y;
		} grid;

		struct {
			unsigned int number;
			int delta;
		} encoder;

		struct {
			unsigned int sensor;
			int x;
			int y;
			int z;
		} tilt;
	};
};

monome_t *monome_open(const char *monome_device, ...);
void monome_close(monome_t *monome);

void monome_set_rotation(monome_t *monome, monome_rotate_t cable);
monome_rotate_t monome_get_rotation(monome_t *monome);

const char *monome_get_serial(monome_t *monome);
const char *monome_get_devpath(monome_t *monome);
const char *monome_get_friendly_name(monome_t *monome);
const char *monome_get_proto(monome_t *monome);
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

/**
 * led grid commands
 */
int monome_led_set(monome_t *monome, unsigned int x, unsigned int y,
                   unsigned int on);
int monome_led_on(monome_t *monome, unsigned int x, unsigned int y);
int monome_led_off(monome_t *monome, unsigned int x, unsigned int y);
int monome_led_all(monome_t *monome, unsigned int status);
int monome_led_map(monome_t *monome, unsigned int x_off, unsigned int y_off,
                   const uint8_t *data);
int monome_led_col(monome_t *monome, unsigned int x, unsigned int y_off,
                   size_t count, const uint8_t *col_data);
int monome_led_row(monome_t *monome, unsigned int x_off, unsigned int y,
                   size_t count, const uint8_t *row_data);
int monome_led_intensity(monome_t *monome, unsigned int brightness);

int monome_led_level_set(monome_t *monome, unsigned int x, unsigned int y,
                         unsigned int level);
int monome_led_level_all(monome_t *monome, unsigned int level);
int monome_led_level_map(monome_t *monome, unsigned int x_off,
                         unsigned int y_off, const uint8_t *data);
int monome_led_level_row(monome_t *monome, unsigned int x_off,
                         unsigned int y, size_t count, const uint8_t *data);
int monome_led_level_col(monome_t *monome, unsigned int x, unsigned int y_off,
                         size_t count, const uint8_t *data);

/**
 * led ring commands
 */

int monome_led_ring_set(monome_t *monome, unsigned int ring, unsigned int led,
                        unsigned int level);
int monome_led_ring_all(monome_t *monome, unsigned int ring,
                        unsigned int level);
int monome_led_ring_map(monome_t *monome, unsigned int ring,
                        const uint8_t *levels);
int monome_led_ring_range(monome_t *monome, unsigned int ring,
                          unsigned int start, unsigned int end,
                          unsigned int level);

/**
 * tilt commands
 */
int monome_tilt_enable(monome_t *monome, unsigned int sensor);
int monome_tilt_disable(monome_t *monome, unsigned int sensor);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* defined _MONOME_H */
