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

void left_cb(monome_t *monome, uint *x, uint *y) {
	return;
}

void bottom_output_cb(monome_t *monome, uint *x, uint *y) {
	uint t = *x;

	*x = COLS(monome) - *y;
	*y = t;
}

void bottom_input_cb(monome_t *monome, uint *x, uint *y) {
	uint t = *x;

	*x = *y;
	*y = (COLS(monome) - t) % (COLS(monome) + 1);
}

void right_output_cb(monome_t *monome, uint *x, uint *y) {
	*x = ROWS(monome) - *x;
	*y = COLS(monome) - *y;
}

void right_input_cb(monome_t *monome, uint *x, uint *y) {
	*x = (ROWS(monome) - *x) % (ROWS(monome) + 1);
	*y = (COLS(monome) - *y) % (COLS(monome) + 1);
}

void top_output_cb(monome_t *monome, uint *x, uint *y) {
	uint t = *x;

	*x = *y;
	*y = ROWS(monome) - t;
}

void top_input_cb(monome_t *monome, uint *x, uint *y) {
	uint t = *x;

	*x = (ROWS(monome) - *y) % (ROWS(monome) + 1);
	*y = t;
}

monome_rotspec_t rotation[4] = {
	[MONOME_CABLE_LEFT] = {
		.output_cb = left_cb,
		.input_cb = left_cb,
		.flags    = 0,
	},
	
	[MONOME_CABLE_BOTTOM] = {
		.output_cb = bottom_output_cb,
		.input_cb = bottom_input_cb,
		.flags    = ROW_COL_SWAP | COL_REVBITS
	},

	[MONOME_CABLE_RIGHT] = {
		.output_cb = right_output_cb,
		.input_cb = right_input_cb,
		.flags    = ROW_REVBITS | COL_REVBITS
	},

	[MONOME_CABLE_TOP] = {
		.output_cb = top_output_cb,
		.input_cb = top_input_cb,
		.flags    = ROW_COL_SWAP | ROW_REVBITS
	},
};
