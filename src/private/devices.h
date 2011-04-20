/**
 * Copyright (c) 2011 William Light <wrl@illest.net>
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

#include "internal.h"

static monome_devmap_t mapping[] = {
	{"m64-%d",  "series", {8, 8},   "monome 64" , NO_QUIRKS},
	{"m128-%d", "series", {16, 8},  "monome 128", NO_QUIRKS},
	{"m256-%d", "series", {16, 16}, "monome 256", NO_QUIRKS},

	{"mk%d",    "series", {16, 16},  "monome kit", NO_QUIRKS},

	{"m40h%d",  "40h",    {8, 8},   "monome 40h", NO_QUIRKS},
	{"a40h-%d", "40h",    {8, 8},   "arduinome" , QUIRK_57600_BAUD},

	/* determine device dimensions in initialization */
	{"m%d",     "mext",   {0, 0},   "monome i2c", NO_QUIRKS},

	{NULL}
};
