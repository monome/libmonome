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

#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <glob.h>
#include <assert.h>

#include <monome.h>
#include "monome_internal.h"

#define FTDI_PATH "/sys/bus/usb/drivers/ftdi_sio"
#define MAX_LENGTH 128

extern monome_device_mapping_t mapping[];

static char *get_serial(int bus, int device) {
	char *filestr, buf[MAX_LENGTH], *serial;
	int len, serial_fd;

	serial = NULL;

	if( asprintf(&filestr, "/sys/bus/usb/devices/%d-%d/serial", bus, device) < 0 )
		return NULL;

	if( (serial_fd = open(filestr, O_RDONLY)) < 0 )
		goto err;

	len = read(serial_fd, buf, sizeof(buf));
	if( len < 0 )
		goto file_err;

	serial = strndup(buf, len - 1);

file_err:
	close(serial_fd);

err:
	free(filestr);
	return serial;
}

static int get_monome_information_from_devname(monome_t *monome, const char *path) {
	monome_device_mapping_t *c;
	monome_device_t model = 0;
	int bus, device, serial;
	glob_t gb;
	char *buf;

	assert(monome);
	assert(path);

	if( *path == '/' )
		path++;

	path = strchr(path, '/') + 1;

	asprintf(&buf, FTDI_PATH "/*/%s", path);
	glob(buf, 0, NULL, &gb);
	free(buf);

	if( !gb.gl_pathc )
		return 1;

	sscanf(*gb.gl_pathv, FTDI_PATH "/%d-%d", &bus, &device);
	globfree(&gb);

	buf = get_serial(bus, device);

	for( c = mapping; c->serial; c++ ) {
		if( !sscanf(buf, c->serial, &serial) )
			continue;

		model = c->model;
		break;
	}

	if( !model ) {
		/* unrecognized device, go with lowest common denominator */
		monome->model = MONOME_DEVICE_40h;
		return 1;
	}

	monome->model  = model;
	monome->serial = buf;
	monome->device = strdup(path);

	return 0;
}

#include "posix.inc"
