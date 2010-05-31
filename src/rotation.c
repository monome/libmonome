/*
 * This file is part of libmonome.
 * libmonome is copyright 2007-2010 will light <visinin@gmail.com>
 *
 * libmonome is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 *
 */

#include <stdio.h>
#include <stdint.h>

#include <monome.h>
#include "internal.h"

/* faster than using the global libmonome functions
   also, the global functions are 1-indexed, these are 0-indexed */
#define ROWS(monome) (monome->rows - 1)
#define COLS(monome) (monome->cols - 1)

/**
 * public
 */

/* you may notice the gratituous use of modulo when translating input
   coordinates...this is because it's possible to translate into negatives
   when pretending a bigger monome (say, a 256) is a smaller monome (say,
   a 128).  because we're using unsigned integers, this will cause a
   wrap-around into some very big numbers, which makes several of the example
   programs segfault (simple.c, in particular).

   while this bug is arguably contrived, I'd rather pay the minute
   computational cost here and avoid causing trouble in application code. */

static void left_cb(monome_t *monome, uint *x, uint *y) {
	return;
}

static void left_frame_cb(monome_t *monome, uint *quadrant, uint8_t *frame_data) {
	return;
}

static void bottom_output_cb(monome_t *monome, uint *x, uint *y) {
	uint t = *x;

	*x = COLS(monome) - *y;
	*y = t;
}

static void bottom_input_cb(monome_t *monome, uint *x, uint *y) {
	uint t = *x;

	*x = *y;
	*y = (COLS(monome) - t) % (COLS(monome) + 1);
}

static void bottom_frame_cb(monome_t *monome, uint *quadrant, uint8_t *frame_data) {
}

static void right_output_cb(monome_t *monome, uint *x, uint *y) {
	*x = ROWS(monome) - *x;
	*y = COLS(monome) - *y;
}

static void right_input_cb(monome_t *monome, uint *x, uint *y) {
	*x = (ROWS(monome) - *x) % (ROWS(monome) + 1);
	*y = (COLS(monome) - *y) % (COLS(monome) + 1);
}

static void right_frame_cb(monome_t *monome, uint *quadrant, uint8_t *frame_data) {
	uint64_t x = *((uint64_t *) frame_data);

	/* straightforward 64bit integer reversal...
	   might not work on embedded platforms? */

	x = x >> 32 | x << 32;
	x = (x & 0xFFFF0000FFFF0000) >> 16 | (x & 0x0000FFFF0000FFFF) << 16;
	x = (x & 0xFF00FF00FF00FF00) >> 8  | (x & 0x00FF00FF00FF00FF) << 8;
	x = (x & 0xF0F0F0F0F0F0F0F0) >> 4  | (x & 0x0F0F0F0F0F0F0F0F) << 4;
	x = (x & 0xCCCCCCCCCCCCCCCC) >> 2  | (x & 0x3333333333333333) << 2;
	x = (x & 0xAAAAAAAAAAAAAAAA) >> 1  | (x & 0x5555555555555555) << 1;

	*((uint64_t *) frame_data) = x;
	*quadrant = (3 - *quadrant) & 0x3;
}

static void top_output_cb(monome_t *monome, uint *x, uint *y) {
	uint t = *x;

	*x = *y;
	*y = ROWS(monome) - t;
}

static void top_input_cb(monome_t *monome, uint *x, uint *y) {
	uint t = *x;

	*x = (ROWS(monome) - *y) % (ROWS(monome) + 1);
	*y = t;
}

static void top_frame_cb(monome_t *monome, uint *quadrant, uint8_t *frame_data) {
}

monome_rotspec_t rotation[4] = {
	[MONOME_CABLE_LEFT] = {
		.output_cb = left_cb,
		.input_cb = left_cb,
		.frame_cb = left_frame_cb,

		.flags    = 0,
	},
	
	[MONOME_CABLE_BOTTOM] = {
		.output_cb = bottom_output_cb,
		.input_cb = bottom_input_cb,
		.frame_cb = bottom_frame_cb,

		.flags    = ROW_COL_SWAP | COL_REVBITS
	},

	[MONOME_CABLE_RIGHT] = {
		.output_cb = right_output_cb,
		.input_cb = right_input_cb,
		.frame_cb = right_frame_cb,

		.flags    = ROW_REVBITS | COL_REVBITS
	},

	[MONOME_CABLE_TOP] = {
		.output_cb = top_output_cb,
		.input_cb = top_input_cb,
		.frame_cb = top_frame_cb,

		.flags    = ROW_COL_SWAP | ROW_REVBITS
	},
};
