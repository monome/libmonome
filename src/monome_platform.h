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

#ifndef _MONOME_PLATFORM_H
#define _MONOME_PLATFORM_H

#include "monome_internal.h"

int monome_platform_open(monome_t *monome, const char *dev);
int monome_platform_close(monome_t *monome);

ssize_t monome_platform_write(monome_t *monome, const uint8_t *buf, ssize_t bufsize);
ssize_t monome_platform_read(monome_t *monome, uint8_t *buf, ssize_t bufsize);
	
#endif
