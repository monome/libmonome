/*
 * Copyright (c) 2007-2010, William Light <will@visinin.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
typedef void (*monome_frame_cb)(monome_t *, uint *quadrant, uint8_t *frame_data);

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

	char *serial;
	char *device;
	int rows, cols;

	struct termios ot;
	int fd;

	monome_callback_t handlers[3];
	monome_cable_t orientation;

	int  (*open)(monome_t *monome, const char *dev, va_list args);
	int  (*close)(monome_t *monome);
	void (*free)(monome_t *monome);

	int  (*next_event)(monome_t *monome, monome_event_t *event);

	int  (*clear)(monome_t *monome, monome_clear_status_t status);
	int  (*intensity)(monome_t *monome, uint brightness);
	int  (*mode)(monome_t *monome, monome_mode_t mode);

	int  (*led_on)(monome_t *monome, uint x, uint y);
	int  (*led_off)(monome_t *monome, uint x, uint y);
	int  (*led_col)(monome_t *monome, uint col, size_t count, const uint8_t *data);
	int  (*led_row)(monome_t *monome, uint row, size_t count, const uint8_t *data);
	int  (*led_frame)(monome_t *monome, uint quadrant, const uint8_t *frame_data);
};

#endif
