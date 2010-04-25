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

#ifndef _MONOME_INTERNAL_H
#define _MONOME_INTERNAL_H

#include <stdarg.h>
#include <stdint.h>
#include <termios.h>

#include <sys/types.h>

#include <monome.h>

typedef struct monome_callback monome_callback_t;
typedef struct monome_devmap monome_devmap_t;

struct monome_callback {
	monome_callback_function_t cb;
	void *data;
};

struct monome_devmap {
	char *friendly;
	monome_model_t model;
	char *sermatch;
	char *proto;
};

struct monome {
	monome_model_t model;
	char *serial;
	char *device;
	
	struct termios ot;
	int fd;
	
	monome_callback_t handlers[3];
	
	int  (*open)(monome_t *monome, const char *dev, va_list args);
	int  (*close)(monome_t *monome);
	void (*free)(monome_t *monome);

	int  (*next_event)(monome_t *monome, monome_event_t *event);

	int  (*clear)(monome_t *monome, monome_clear_status_t status);
	int  (*intensity)(monome_t *monome, unsigned int brightness);
	int  (*mode)(monome_t *monome, monome_mode_t mode);
	
	int  (*led_on)(monome_t *monome, unsigned int x, unsigned int y);
	int  (*led_off)(monome_t *monome, unsigned int x, unsigned int y);
	int  (*led_col_8)(monome_t *monome, unsigned int col, unsigned int *col_data);
	int  (*led_row_8)(monome_t *monome, unsigned int row, unsigned int *row_data);
	int  (*led_col_16)(monome_t *monome, unsigned int col, unsigned int *col_data);
	int  (*led_row_16)(monome_t *monome, unsigned int row, unsigned int *row_data);
	int  (*led_frame)(monome_t *monome, unsigned int quadrant, unsigned int *frame_data);
};

#endif
