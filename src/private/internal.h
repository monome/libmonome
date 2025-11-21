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

#ifndef MONOME_INTERNAL_H
#define MONOME_INTERNAL_H

#include <stdarg.h>
#include <stdint.h>

#include <sys/types.h>

#include <monome.h>

typedef unsigned int uint_t;

typedef enum {
	NO_QUIRKS        = 0,
	QUIRK_57600_BAUD = 0x1,
} monome_device_quirks_t;

typedef struct monome_callback monome_callback_t;
typedef struct monome_rotspec monome_rotspec_t;
typedef struct monome_devmap monome_devmap_t;

typedef struct monome_led_functions monome_led_functions_t;
typedef struct monome_led_level_functions monome_led_level_functions_t;
typedef struct monome_led_ring_functions monome_led_ring_functions_t;
typedef struct monome_tilt_functions monome_tilt_functions_t;

typedef void (*monome_coord_cb_t)(monome_t *, uint_t *x, uint_t *y);
typedef void (*monome_map_cb_t)(monome_t *, uint8_t *data);
typedef void (*monome_level_map_cb_t)(monome_t *, uint8_t *dest,
                                      const uint8_t *src);

typedef monome_t *(*monome_proto_new_func_t)(void);

struct monome_callback {
	monome_event_callback_t cb;
	void *data;
};

struct monome_devmap {
	char *sermatch;
	char *proto;
	struct {
		int cols, rows;
	} dimensions;
	char *friendly;
	monome_device_quirks_t quirks;
};

struct monome_rotspec {
	monome_coord_cb_t output_cb;
	monome_coord_cb_t input_cb;
	monome_map_cb_t map_cb;
	monome_level_map_cb_t level_map_cb;

	enum {
		ROW_COL_SWAP    = 0x1,
		ROW_REVBITS     = 0x2,
		COL_REVBITS     = 0x4
	} flags;
};

/**
 * subsystem functions
 */

struct monome_led_functions {
	int (*set)(monome_t *monome, uint_t x, uint_t y, uint_t on);
	int (*all)(monome_t *monome, uint_t status);
	int (*map)(monome_t *monome, uint_t x_off, uint_t y_off,
	           const uint8_t *data);
	int (*row)(monome_t *monome, uint_t x_off, uint_t y,
	           size_t count, const uint8_t *data);
	int (*col)(monome_t *monome, uint_t x, uint_t y_off,
	           size_t count, const uint8_t *data);
	int (*intensity)(monome_t *monome, uint_t brightness);
};

struct monome_led_level_functions {
	int (*set)(monome_t *monome, uint_t x, uint_t y, uint_t level);
	int (*all)(monome_t *monome, uint_t level);
	int (*map)(monome_t *monome, uint_t x_off, uint_t y_off,
	           const uint8_t *data);
	int (*row)(monome_t *monome, uint_t x_off, uint_t y,
	           size_t count, const uint8_t *data);
	int (*col)(monome_t *monome, uint_t x, uint_t y_off,
	           size_t count, const uint8_t *data);
};

struct monome_led_ring_functions {
	int (*set)(monome_t *monome, uint_t ring, uint_t led, uint_t level);
	int (*all)(monome_t *monome, uint_t ring, uint_t level);
	int (*map)(monome_t *monome, uint_t ring, const uint8_t *levels);
	int (*range)(monome_t *monome, uint_t ring, uint_t start, uint_t end,
	             uint_t level);
	int (*intensity)(monome_t *monome, uint_t brightness);
};

struct monome_tilt_functions {
	int (*enable)(monome_t *monome, uint_t sensor);
	int (*disable)(monome_t *monome, uint_t sensor);
};

struct monome {
#if !defined(EMBED_PROTOS)
	/* handle for the loaded protocol module */
	void *dl_handle;
#endif

	const char *serial;
	const char *friendly;
	const char *device;
	const char *proto;
	int rows, cols;

	int fd;

	monome_callback_t handlers[MONOME_EVENT_MAX];
	monome_rotate_t rotation;

	int  (*open)(monome_t *monome, const char *dev, const char *serial,
				 const monome_devmap_t *, va_list args);
	int  (*close)(monome_t *monome);
	void (*free)(monome_t *monome);

	int  (*next_event)(monome_t *monome, monome_event_t *event);

	monome_led_functions_t *led;
	monome_led_level_functions_t *led_level;
	monome_led_ring_functions_t *led_ring;
	monome_tilt_functions_t *tilt;
};

#endif /* defined MONOME_INTERNAL_H */
