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

extern monome_rotspec_t rotation[4];

#define ORIENTATION(monome) (rotation[monome->orientation])
#define ROTATE_COORDS(monome, x, y) (ORIENTATION(monome).output_cb(monome, &x, &y))
#define UNROTATE_COORDS(monome, x, y) (ORIENTATION(monome).input_cb(monome, &x, &y))

#define REVERSE_BYTE(x) ((uint) (((x * 0x0802) & 0x22110) | ((x * 0x8020) & 0x88440)) * 0x10101 >> 16)
