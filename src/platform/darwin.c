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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <monome.h>
#include "monome_internal.h"

char *monome_platform_get_dev_serial(monome_t *monome, const char *path) {
	monome_device_mapping_t *c;
	monome_device_t model = 0;
	int serialnum;
	char *serial;

	assert(path);

	/* osx serial paths are of the form
	   /dev/tty.usbserial-<device serial>

	   we'll locate to the first hyphen */

	serial = strchr(path, '-') + 1;
	return strdup(serial);
}

#include "posix.inc"
