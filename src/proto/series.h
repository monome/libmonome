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

#include "monome.h"
#include "internal.h"

typedef enum {
	/* input (from device) */

	PROTO_SERIES_BUTTON_DOWN         = 0x00,
	PROTO_SERIES_BUTTON_UP           = 0x10,
	PROTO_SERIES_AUX_INPUT           = 0xE0,

	/* output (to device) */

	PROTO_SERIES_LED_ON              = 0x20,
	PROTO_SERIES_LED_OFF             = 0x30,
	PROTO_SERIES_LED_ROW_8           = 0x40,
	PROTO_SERIES_LED_COL_8           = 0x50,
	PROTO_SERIES_LED_ROW_16          = 0x60,
	PROTO_SERIES_LED_COL_16          = 0x70,
	PROTO_SERIES_LED_FRAME           = 0x80,
	PROTO_SERIES_CLEAR               = 0x90,
	PROTO_SERIES_INTENSITY           = 0xA0,
	PROTO_SERIES_MODE                = 0xB0,
	PROTO_SERIES_AUX_PORT_ACTIVATE   = 0xC0,
	PROTO_SERIES_AUX_PORT_DEACTIVATE = 0xD0
} proto_series_message_t;

/* clearing statuses (argument to PROTO_SERIES_CLEAR output command) */

typedef enum {
	PROTO_SERIES_CLEAR_OFF           = 0x00,
	PROTO_SERIES_CLEAR_ON            = 0x01
} proto_series_clear_status_t;

/* modes (argument to the PROTO_SERIES_MODE output command) */

typedef enum {
	PROTO_SERIES_MODE_NORMAL         = 0x00,
	PROTO_SERIES_MODE_TEST           = 0x01,
	PROTO_SERIES_MODE_SHUTDOWN       = 0x02
} proto_series_mode_t;
