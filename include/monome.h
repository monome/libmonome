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

#ifndef _MONOME_H
#define _MONOME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <termios.h>

#include <sys/types.h>

typedef enum {
	MONOME_BUTTON_UP           = 0x00,
	MONOME_BUTTON_DOWN         = 0x01,
	MONOME_AUX_INPUT           = 0x02
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

/* cable orientation */

typedef enum {
	MONOME_CABLE_LEFT          = 0,
	MONOME_CABLE_BOTTOM        = 1,
	MONOME_CABLE_RIGHT         = 2,
	MONOME_CABLE_TOP           = 3
} monome_cable_t;
	
/* devices and their dimensions */

typedef enum {	
	MONOME_DEVICE_256          = 0xFF,
	MONOME_DEVICE_128          = 0x7F,
	MONOME_DEVICE_64           = 0x77,
	MONOME_DEVICE_40h          = 0x77
} monome_model_t;

typedef struct monome_event monome_event_t;
typedef struct monome monome_t; /* opaque data type */

typedef void (*monome_callback_function_t)(const monome_event_t *event, void *data);

struct monome_event {
	monome_t *monome;
	monome_event_type_t event_type;
	uint x;
	uint y;
};

monome_t *monome_open(const char *monome_device, ...);
void monome_close(monome_t *monome);

int monome_get_rows(monome_t *monome);
int monome_get_cols(monome_t *monome);

void monome_register_handler(monome_t *monome, uint event_type, monome_callback_function_t, void *user_data);
void monome_unregister_handler(monome_t *monome, uint event_type);
void monome_main_loop(monome_t *monome);
int monome_next_event(monome_t *monome);

int monome_clear(monome_t *monome, monome_clear_status_t status);
int monome_intensity(monome_t *monome, uint brightness);
int monome_mode(monome_t *monome, monome_mode_t mode);

int monome_led_on(monome_t *monome, uint x, uint y);
int monome_led_off(monome_t *monome, uint x, uint y);
int monome_led_col_8(monome_t *monome, uint col, uint *col_data);
int monome_led_row_8(monome_t *monome, uint row, uint *row_data);
int monome_led_col_16(monome_t *monome, uint col, uint *col_data);
int monome_led_row_16(monome_t *monome, uint row, uint *row_data);
int monome_led_frame(monome_t *monome, uint quadrant, uint *frame_data);

#ifdef __cplusplus
} /* extern "C" */
#endif
#endif /* defined _MONOME_H */
