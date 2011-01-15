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

#include "internal.h"

extern monome_rotspec_t rotspec[4];

#define ROTSPEC(monome) (rotspec[monome->rotation])
#define ROTATE_COORDS(monome, x, y) (ROTSPEC(monome).output_cb(monome, &x, &y))
#define UNROTATE_COORDS(monome, x, y) (ROTSPEC(monome).input_cb(monome, &x, &y))

#define REVERSE_BYTE(x) ((uint_t) (((x * 0x0802) & 0x22110) | ((x * 0x8020) & 0x88440)) * 0x10101 >> 16)
