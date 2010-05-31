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

	PROTO_40h_BUTTON_DOWN         = 0x01,
	PROTO_40h_BUTTON_UP           = 0x00,
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
