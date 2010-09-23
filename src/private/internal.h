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

#ifndef _MONOME_INTERNAL_H
#define _MONOME_INTERNAL_H

#include <stdarg.h>
#include <stdint.h>
#include <termios.h>

#include <sys/types.h>

#include <monome.h>

typedef struct monome_callback monome_callback_t;
typedef struct monome_rotspec monome_rotspec_t;
typedef struct monome_devmap monome_devmap_t;

typedef void (*monome_coord_cb)(monome_t *, uint *x, uint *y);
typedef void (*monome_frame_cb)(monome_t *, uint *quadrant,
								uint8_t *frame_data);

struct monome_callback {
	monome_event_callback_t cb;
	void *data;
};

struct monome_devmap {
	char *sermatch;
	char *proto;
	struct {
		int rows, cols;
	} dimensions;
	char *friendly;
};

struct monome_rotspec {
	monome_coord_cb output_cb;
	monome_coord_cb input_cb;
	monome_frame_cb frame_cb;

	enum {
		ROW_COL_SWAP    = 0x1,
		ROW_REVBITS     = 0x2,
		COL_REVBITS     = 0x4
	} flags;
};

struct monome {
	void *dl_handle;

	const char *serial;
	const char *device;
	int rows, cols;

	struct termios ot;
	int fd;

	monome_callback_t handlers[3];
	monome_rotate_t rotation;

	int  (*open)(monome_t *monome, const char *dev, const char *serial,
				 const monome_devmap_t *, va_list args);
	int  (*close)(monome_t *monome);
	void (*free)(monome_t *monome);

	int  (*next_event)(monome_t *monome, monome_event_t *event);

	int  (*clear)(monome_t *monome, monome_clear_status_t status);
	int  (*intensity)(monome_t *monome, uint brightness);
	int  (*mode)(monome_t *monome, monome_mode_t mode);

	int  (*led)(monome_t *monome, uint x, uint y, uint on);
	int  (*led_col)(monome_t *monome, uint col, size_t count, const uint8_t *data);
	int  (*led_row)(monome_t *monome, uint row, size_t count, const uint8_t *data);
	int  (*led_frame)(monome_t *monome, uint quadrant, const uint8_t *frame_data);
};

#endif
