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

#ifndef _MONOME_INTERNAL_H
#define _MONOME_INTERNAL_H

#include <stdint.h>
#include <termios.h>

#include <sys/types.h>

#include <monome.h>

typedef struct monome_callback monome_callback_t;

struct monome_callback {
	monome_callback_function_t cb;
	void *data;
	struct monome_callback *next;
};

struct monome {
	char *dev;
	monome_device_t model;
	
	struct termios ot;
	int fd;
	
	monome_callback_t *handlers[2];
	
	/* int monome_protocol_populate_event(monome_event_t *event, const uint8_t *buf, const ssize_t buf_size); */
	int (*populate_event)(monome_event_t *event, const uint8_t *buf, const ssize_t buf_size);

	int (*clear)(monome_t *monome, monome_clear_status_t status);
	int (*intensity)(monome_t *monome, unsigned int brightness);
	int (*mode)(monome_t *monome, monome_mode_t mode);
	
	int (*led_on)(monome_t *monome, unsigned int x, unsigned int y);
	int (*led_off)(monome_t *monome, unsigned int x, unsigned int y);
	int (*led_col_8)(monome_t *monome, unsigned int col, unsigned int *col_data);
	int (*led_row_8)(monome_t *monome, unsigned int row, unsigned int *row_data);
	int (*led_col_16)(monome_t *monome, unsigned int col, unsigned int *col_data);
	int (*led_row_16)(monome_t *monome, unsigned int row, unsigned int *row_data);
	int (*led_frame)(monome_t *monome, unsigned int quadrant, unsigned int *frame_data);
};

#endif
