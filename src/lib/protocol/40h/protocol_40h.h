/*
 * This file is part of libmonome.
 * libmonome is copyright 2007, 2008 will light <visinin@gmail.com>
 *
 * libmonome is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 *
 */

#include "monome.h"
#include "monome_internal.h"

typedef enum {
	/* input (from device) */

	PROTO_40h_BUTTON_DOWN         = 0x00,
	PROTO_40h_BUTTON_UP           = 0x01,
	PROTO_40h_AUX_INPUT           = 0x10,
	
	/* output (to device) */

	PROTO_40h_LED_ON              = 0x21,
	PROTO_40h_LED_OFF             = 0x20,
	PROTO_40h_INTENSITY           = 0x30,
	PROTO_40h_LED_TEST            = 0x40,
	PROTO_40h_ADC_ENABLE          = 0x50,
	PROTO_40h_SHUTDOWN            = 0x60,
	PROTO_40h_LED_ROW             = 0x70,
	PROTO_40h_LED_COL             = 0x80,
} proto_40h_message_t;


typedef struct monome_40h monome_40h_t;

struct monome_40h {
	monome_t parent;
	monome_mode_t mode;
};
