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

#include <stdio.h>

#include "monome.h"

int monome_device_open(monome_t *monome) {
	printf("libmonome was compiled with the dummy platform and, hence, effectively does nothing.\n"
		   "please recompile for any functionality!\n");
	
	return 1;
}

ssize_t monome_device_write(monome_t *monome, const uint8_t *buf, ssize_t bufsize) {
	return 0;
}

ssize_t monome_device_read(monome_t *monome, uint8_t *buf, ssize_t count) {
	return 0;
}
