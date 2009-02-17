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
};

#endif
