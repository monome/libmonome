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

#include <lo/lo.h>

#include "monome.h"
#include "internal.h"

typedef struct monome_osc monome_osc_t;

struct monome_osc {
	monome_t parent;

	lo_server server;
	lo_address outgoing;
	char *prefix;

	int have_event;
	monome_event_t *e_ptr;

	char *clear_str;
	char *intensity_str;
	char *mode_str;

	char *led_str;
	char *led_row_str;
	char *led_col_str;
	char *frame_str;
};
