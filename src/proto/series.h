/*
 * Copyright (c) 2007-2010, William Light <will@visinin.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
