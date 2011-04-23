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

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <monome.h>
#include "internal.h"

#define ROWS(monome) (monome_get_rows(monome) - 1)
#define COLS(monome) (monome_get_cols(monome) - 1)

/* you may notice the gratituous use of modulo when translating input
   coordinates...this is because it's possible to translate into negatives
   when pretending a bigger monome (say, a 256) is a smaller monome (say,
   a 128). because we're using unsigned integers, this will cause a wrap-around
   into some very big numbers, which makes several of the example programs
   segfault (simple.c, in particular).

   while this bug is arguably contrived, I'd rather pay the minute
   computational cost here and avoid causing trouble in application code. */

/**
 * 0 degrees
 */

static void r0_cb(monome_t *monome, uint_t *x, uint_t *y) {
	return;
}

static void r0_map_cb(monome_t *monome, uint8_t *data) {
	return;
}

static void r0_level_map_cb(monome_t *monome, uint8_t *dst,
                            const uint8_t *src) {
	memcpy(dst, src, 64);
}

/**
 * 90 degrees
 */

static void r90_output_cb(monome_t *monome, uint_t *x, uint_t *y) {
	uint_t t = *x;

	*x = *y;
	*y = COLS(monome) - t;
}

static void r90_input_cb(monome_t *monome, uint_t *x, uint_t *y) {
	uint_t t = *x;

	*x = (COLS(monome) - *y) % (COLS(monome) + 1);
	*y = t;
}

static void r90_map_cb(monome_t *monome, uint8_t *data) {
	/* this is an algorithm for rotation of a bit matrix by 90 degrees.
	   in the case of r270_map_cb, the rotation is clockwise, in the case
	   of r90_map_cb it is counter-clockwise.

	   the matrix is made up of an array of 8 bytes, which, laid out
	   contiguously in memory, can be treated as a 64 bit integer, which I've
	   opted to do here. this allows rotation to be accomplished solely with
	   bitwise operations.

	   on 64 bit architectures, we treat data as a 64 bit integer, on 32
	   bit architectures we treat it as two 32 bit integers.

	   inspired by "hacker's delight" by henry s. warren
	   see section 7-3 "transposing a bit matrix" */

#ifdef __LP64__
	uint64_t t, x = *((uint64_t *) data);

# define swap(f, c)\
	t = (x ^ (x << f)) & c; x ^= t ^ (t >> f);

# ifdef LM_BIG_ENDIAN
	swap(8, 0xFF00FF00FF00FF00LLU);
	swap(7, 0x5500550055005500LLU);

	swap(16, 0xFFFF0000FFFF0000LLU);
	swap(14, 0x3333000033330000LLU);

	swap(32, 0xFFFFFFFF00000000LLU);
	swap(28, 0x0F0F0F0F00000000LLU);
# else
	swap(8, 0xFF00FF00FF00FF00LLU);
	swap(9, 0xAA00AA00AA00AA00LLU);

	swap(16, 0xFFFF0000FFFF0000LLU);
	swap(18, 0xCCCC0000CCCC0000LLU);

	swap(32, 0xFFFFFFFF00000000LLU);
	swap(36, 0xF0F0F0F000000000LLU);
# endif /* defined LM_BIG_ENDIAN */
# undef swap

	*((uint64_t *) data) = x;
#else /* __LP64__ */
	uint32_t x, y, t;

	x = *((uint32_t *) data);
	y = *(((uint32_t *) data) + 1);
	t = 0;

# define swap(x, f, c)\
	t = (x ^ (x << f)) & c; x ^= t ^ (t >> f);

# ifdef LM_BIG_ENDIAN
	swap(x, 8, 0xFF00FF00);
	swap(x, 7, 0x55005500);

	swap(x, 16, 0xFFFF0000);
	swap(x, 14, 0x33330000);

	swap(y, 8, 0xFF00FF00);
	swap(y, 7, 0x55005500);

	swap(y, 16, 0xFFFF0000);
	swap(y, 14, 0x33330000);
# else
	swap(x, 8, 0xFF00FF00);
	swap(x, 9, 0xAA00AA00);

	swap(x, 16, 0xFFFF0000);
	swap(x, 18, 0xCCCC0000);

	swap(y, 8, 0xFF00FF00);
	swap(y, 9, 0xAA00AA00);

	swap(y, 16, 0xFFFF0000);
	swap(y, 18, 0xCCCC0000);
# endif /* defined LM_BIG_ENDIAN */
# undef swap

	*((uint32_t *) data) = ((x & 0xF0F0F0F0) >> 4) | (y & 0xF0F0F0F0);
	*(((uint32_t *) data) + 1) = (x & 0x0F0F0F0F) | ((y & 0x0F0F0F0F) << 4);
#endif /* __LP64__ */
}

static void r90_level_map_cb(monome_t *monome, uint8_t *dst,
                             const uint8_t *src) {
	int i;

	for( i = 0; i < 64; i++ )
		dst[i] = src[(7 - (i >> 3)) + ((i & 7) << 3)];
}

/**
 * 180 degrees
 */

static void r180_output_cb(monome_t *monome, uint_t *x, uint_t *y) {
	*x = COLS(monome) - *x;
	*y = ROWS(monome) - *y;
}

static void r180_input_cb(monome_t *monome, uint_t *x, uint_t *y) {
	*x = (COLS(monome) - *x) % (COLS(monome) + 1);
	*y = (ROWS(monome) - *y) % (ROWS(monome) + 1);
}

static void r180_map_cb(monome_t *monome, uint8_t *data) {
	/* integer reversal. */

#ifdef __LP64__
	uint64_t x = *((uint64_t *) data);

	x = x >> 32 | x << 32;
	x = (x & 0xFFFF0000FFFF0000LLU) >> 16 | (x & 0x0000FFFF0000FFFFLLU) << 16;
	x = (x & 0xFF00FF00FF00FF00LLU) >> 8  | (x & 0x00FF00FF00FF00FFLLU) << 8;
	x = (x & 0xF0F0F0F0F0F0F0F0LLU) >> 4  | (x & 0x0F0F0F0F0F0F0F0FLLU) << 4;
	x = (x & 0xCCCCCCCCCCCCCCCCLLU) >> 2  | (x & 0x3333333333333333LLU) << 2;
	x = (x & 0xAAAAAAAAAAAAAAAALLU) >> 1  | (x & 0x5555555555555555LLU) << 1;

	*((uint64_t *) data) = x;
#else /* __LP64__ */
	uint32_t x, y;

	x = *((uint32_t *) data);
	y = *(((uint32_t *) data) + 1);

	x = x >> 16 | x << 16;
	x = (x & 0xFF00FF00) >> 8  | (x & 0x00FF00FF) << 8;
	x = (x & 0xF0F0F0F0) >> 4  | (x & 0x0F0F0F0F) << 4;
	x = (x & 0xCCCCCCCC) >> 2  | (x & 0x33333333) << 2;
	x = (x & 0xAAAAAAAA) >> 1  | (x & 0x55555555) << 1;

	y = y >> 16 | y << 16;
	y = (y & 0xFF00FF00) >> 8  | (y & 0x00FF00FF) << 8;
	y = (y & 0xF0F0F0F0) >> 4  | (y & 0x0F0F0F0F) << 4;
	y = (y & 0xCCCCCCCC) >> 2  | (y & 0x33333333) << 2;
	y = (y & 0xAAAAAAAA) >> 1  | (y & 0x55555555) << 1;

	*((uint32_t *) data) = y;
	*(((uint32_t *) data) + 1) = x;
#endif /* defined __LP64__ */
}

static void r180_level_map_cb(monome_t *monome, uint8_t *dst,
                              const uint8_t *src) {
	int i;

	for( i = 0; i < 64; i++ )
		dst[63 - i] = src[i];
}

/**
 * 270 degrees
 */

static void r270_output_cb(monome_t *monome, uint_t *x, uint_t *y) {
	uint_t t = *x;

	*x = ROWS(monome) - *y;
	*y = t;
}

static void r270_input_cb(monome_t *monome, uint_t *x, uint_t *y) {
	uint_t t = *x;

	*x = *y;
	*y = (ROWS(monome) - t) % (ROWS(monome) + 1);
}

static void r270_map_cb(monome_t *monome, uint8_t *data) {
	/* see r90_map_cb for a brief explanation */

#ifdef __LP64__
	uint64_t t, x = *((uint64_t *) data);

# define swap(f, c)\
	t = (x ^ (x << f)) & c; x ^= t ^ (t >> f);

# ifdef LM_BIG_ENDIAN
	swap(8, 0xFF00FF00FF00FF00LLU);
	swap(9, 0xAA00AA00AA00AA00LLU);

	swap(16, 0xFFFF0000FFFF0000LLU);
	swap(18, 0xCCCC0000CCCC0000LLU);

	swap(32, 0xFFFFFFFF00000000LLU);
	swap(36, 0xF0F0F0F000000000LLU);
# else
	swap(8, 0xFF00FF00FF00FF00LLU);
	swap(7, 0x5500550055005500LLU);

	swap(16, 0xFFFF0000FFFF0000LLU);
	swap(14, 0x3333000033330000LLU);

	swap(32, 0xFFFFFFFF00000000LLU);
	swap(28, 0x0F0F0F0F00000000LLU);
# endif /* defined LM_BIG_ENDIAN */
# undef swap

	*((uint64_t *) data) = x;
#else /* __LP64__ */
	uint32_t x, y, t;

	x = *((uint32_t *) data);
	y = *(((uint32_t *) data) + 1);
	t = 0;

# define swap(x, f, c)\
	t = (x ^ (x << f)) & c; x ^= t ^ (t >> f);

# ifdef LM_BIG_ENDIAN
	swap(x, 8, 0xFF00FF00);
	swap(x, 9, 0xAA00AA00);

	swap(x, 16, 0xFFFF0000);
	swap(x, 18, 0xCCCC0000);

	swap(y, 8, 0xFF00FF00);
	swap(y, 9, 0xAA00AA00);

	swap(y, 16, 0xFFFF0000);
	swap(y, 18, 0xCCCC0000);
# else
	swap(x, 8, 0xFF00FF00);
	swap(x, 7, 0x55005500);

	swap(x, 16, 0xFFFF0000);
	swap(x, 14, 0x33330000);

	swap(y, 8, 0xFF00FF00);
	swap(y, 7, 0x55005500);

	swap(y, 16, 0xFFFF0000);
	swap(y, 14, 0x33330000);
# endif /* defined LM_BIG_ENDIAN */

	*((uint32_t *) data) = ((x & 0x0F0F0F0F) << 4) | (y & 0x0F0F0F0F);
	*(((uint32_t *) data) + 1) = (x & 0xF0F0F0F0) | ((y & 0xF0F0F0F0) >> 4);
# undef swap

#endif /* defined __LP64__ */
}

static void r270_level_map_cb(monome_t *monome, uint8_t *dst,
                              const uint8_t *src) {
	int i;

	for( i = 0; i < 64; i++ )
		dst[i] = src[(i >> 3) + ((7 - (i & 7)) << 3)];
}

monome_rotspec_t rotspec[4] = {
	[MONOME_ROTATE_0] = {
		.output_cb    = r0_cb,
		.input_cb     = r0_cb,
		.map_cb       = r0_map_cb,
		.level_map_cb = r0_level_map_cb,

		.flags        = 0,
	},
	
	[MONOME_ROTATE_90] = {
		.output_cb    = r90_output_cb,
		.input_cb     = r90_input_cb,
		.map_cb       = r90_map_cb,
		.level_map_cb = r90_level_map_cb,

		.flags        = ROW_COL_SWAP | ROW_REVBITS
	},

	[MONOME_ROTATE_180] = {
		.output_cb    = r180_output_cb,
		.input_cb     = r180_input_cb,
		.map_cb       = r180_map_cb,
		.level_map_cb = r180_level_map_cb,

		.flags        = ROW_REVBITS | COL_REVBITS
	},

	[MONOME_ROTATE_270] = {
		.output_cb    = r270_output_cb,
		.input_cb     = r270_input_cb,
		.map_cb       = r270_map_cb,
		.level_map_cb = r270_level_map_cb,

		.flags        = ROW_COL_SWAP | COL_REVBITS
	},
};
