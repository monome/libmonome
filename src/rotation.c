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

#include <monome.h>
#include "monome_internal.h"

static int left_led_on(monome_t *monome, unsigned int x, unsigned int y) {
	return monome->led_on(monome, x, y);
}

static int left_led_off(monome_t *monome, unsigned int x, unsigned int y) {
	return monome->led_off(monome, x, y);
}

static int left_led_col_8(monome_t *monome, unsigned int col, unsigned int *col_data) {
	return monome->led_col_8(monome, col, col_data);
}

static int left_led_row_8(monome_t *monome, unsigned int row, unsigned int *row_data) {
	return monome->led_row_8(monome, row, row_data);
}

static int left_led_col_16(monome_t *monome, unsigned int col, unsigned int *col_data) {
	return monome->led_col_16(monome, col, col_data);
}

static int left_led_row_16(monome_t *monome, unsigned int row, unsigned int *row_data) {
	return monome->led_row_16(monome, row, row_data);
}

static int left_led_frame(monome_t *monome, unsigned int quadrant, unsigned int *frame_data) {
	return monome->led_frame(monome, quadrant, frame_data);
}

monome_cable_impl_t rotate[4] = {
	[MONOME_CABLE_LEFT] = {
		left_led_on,
		left_led_off,
		left_led_col_8,
		left_led_row_8,
		left_led_col_16,
		left_led_row_16,
		left_led_frame
	}
};
