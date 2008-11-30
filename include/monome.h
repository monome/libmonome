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

#ifndef _MONOME_H
#define _MONOME_H

#include <stdint.h>
#include <termios.h>

#include <sys/types.h>

typedef enum {
	/* input (from device) */

	MONOME_BUTTON_DOWN         = 0x00,
	MONOME_BUTTON_UP           = 0x10,
	MONOME_AUX_INPUT           = 0xE0,
	
	/* output (to device) */

	MONOME_LED_ON              = 0x20,
	MONOME_LED_OFF             = 0x30,
	MONOME_LED_ROW_8           = 0x40,
	MONOME_LED_COL_8           = 0x50,
	MONOME_LED_ROW_16          = 0x60,
	MONOME_LED_COL_16          = 0x70,
	MONOME_LED_FRAME           = 0x80,
	MONOME_CLEAR               = 0x90,
	MONOME_INTENSITY           = 0xA0,
	MONOME_MODE                = 0xB0,
	MONOME_AUX_PORT_ACTIVATE   = 0xC0,
	MONOME_AUX_PORT_DEACTIVATE = 0xD0
} monome_message_t;

/* clearing statuses (argument to MONOME_CLEAR output command) */

typedef enum {
	MONOME_CLEAR_OFF           = 0x00,
	MONOME_CLEAR_ON            = 0x01
} monome_clear_status_t;

/* modes (argument to the MONOME_MODE output command) */

typedef enum {
	MONOME_MODE_NORMAL         = 0x00,
	MONOME_MODE_TEST           = 0x01,
	MONOME_MODE_SHUTDOWN       = 0x02
} monome_mode_t;
	
/* devices and their dimensions (rows are the upper 4 bits, columns are the lower 4) */

typedef enum {	
	MONOME_DEVICE_256          = 0xFF,
	MONOME_DEVICE_128          = 0x7F,
	MONOME_DEVICE_64           = 0x77,
	MONOME_DEVICE_40h          = 0x77
} monome_device_t;


typedef struct monome_event {
	struct monome *monome;
	uint8_t event_type;
	uint8_t x;
	uint8_t y;
} monome_event_t;

typedef struct monome_callback {
	void (*cb)(struct monome_event, void*); /* we do this to avoid compiler warnings about imcomplete types */
	void *data;
	struct monome_callback *next;
} monome_callback_t;

typedef struct monome {
	char *dev;
	monome_device_t model;
	
	struct termios ot;
	int fd;
	
	struct monome_callback *handlers[2];
} monome_t;

typedef void (*monome_callback_function_t)(monome_event_t event, void *data);


monome_t *monome_open(const char *monome_device);
int monome_close(monome_t *monome);

ssize_t monome_clear(monome_t *monome, monome_clear_status_t status);
ssize_t monome_intensity(monome_t *monome, uint8_t brightness);
ssize_t monome_mode(monome_t *monome, monome_mode_t mode);

void monome_register_handler(monome_t *monome, uint8_t event_type, monome_callback_function_t, void *user_data);
void monome_unregister_handler(monome_t *monome, uint8_t event_type, monome_callback_function_t, void *user_data);
void monome_main_loop(monome_t *monome);

ssize_t monome_led_on(monome_t *monome, uint8_t x, uint8_t y);
ssize_t monome_led_off(monome_t *monome, uint8_t x, uint8_t y);
ssize_t monome_led_col_8(monome_t *monome, uint8_t col, uint8_t *col_data);
ssize_t monome_led_row_8(monome_t *monome, uint8_t row, uint8_t *row_data);
ssize_t monome_led_col_16(monome_t *monome, uint8_t col, uint8_t *col_data);
ssize_t monome_led_row_16(monome_t *monome, uint8_t row, uint8_t *row_data);
ssize_t monome_led_frame(monome_t *this, uint8_t quadrant, uint8_t *frame_data);

#endif
