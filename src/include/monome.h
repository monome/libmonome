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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <termios.h>

#include <sys/types.h>

typedef enum {
	MONOME_BUTTON_DOWN         = 0x00,
	MONOME_BUTTON_UP           = 0x10,
	MONOME_AUX_INPUT           = 0x20,
} monome_event_type_t;

/* clearing statuses (argument to monome_clear) */

typedef enum {
	MONOME_CLEAR_OFF           = 0x00,
	MONOME_CLEAR_ON            = 0x01
} monome_clear_status_t;

/* modes (argument to monome_mode) */

typedef enum {
	MONOME_MODE_NORMAL         = 0x00,
	MONOME_MODE_TEST           = 0x01,
	MONOME_MODE_SHUTDOWN       = 0x02
} monome_mode_t;
	
/* devices, their protocol versions, and their dimensions */

typedef enum {
	MONOME_PROTOCOL_SERIES     = 0x0200,
	MONOME_PROTOCOL_40h        = 0x0100
} monome_protocol_version_t;

typedef enum {	
	MONOME_DEVICE_256          = 0x00FF | MONOME_PROTOCOL_SERIES,
	MONOME_DEVICE_128          = 0x007F | MONOME_PROTOCOL_SERIES,
	MONOME_DEVICE_64           = 0x0077 | MONOME_PROTOCOL_SERIES,
	MONOME_DEVICE_40h          = 0x0077 | MONOME_PROTOCOL_40h
} monome_device_t;

typedef struct monome_event monome_event_t;
typedef struct monome monome_t; /* opaque data type */

typedef void (*monome_callback_function_t)(monome_event_t event, void *data);

struct monome_event {
	monome_t *monome;
	monome_event_type_t event_type;
	unsigned int x;
	unsigned int y;
};

monome_t *monome_open(const char *monome_device);
void monome_close(monome_t *monome);

ssize_t monome_clear(monome_t *monome, monome_clear_status_t status);
ssize_t monome_intensity(monome_t *monome, unsigned int brightness);
ssize_t monome_mode(monome_t *monome, monome_mode_t mode);

void monome_register_handler(monome_t *monome, unsigned int event_type, monome_callback_function_t, void *user_data);
void monome_unregister_handler(monome_t *monome, unsigned int event_type, monome_callback_function_t, void *user_data);
void monome_main_loop(monome_t *monome);
ssize_t monome_led_on(monome_t *monome, unsigned int x, unsigned int y);
ssize_t monome_led_off(monome_t *monome, unsigned int x, unsigned int y);
ssize_t monome_led_col_8(monome_t *monome, unsigned int col, unsigned int *col_data);
ssize_t monome_led_row_8(monome_t *monome, unsigned int row, unsigned int *row_data);
ssize_t monome_led_col_16(monome_t *monome, unsigned int col, unsigned int *col_data);
ssize_t monome_led_row_16(monome_t *monome, unsigned int row, unsigned int *row_data);
ssize_t monome_led_frame(monome_t *monome, unsigned int quadrant, unsigned int *frame_data);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* defined _MONOME_H */
