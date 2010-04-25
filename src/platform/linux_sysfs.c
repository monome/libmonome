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

char *monome_platform_get_dev_serial(const char *path) {
	int bus, device;
	glob_t gb;
	char *buf;

	assert(path);

	if( *path == '/' )
		path++;

	path = strchr(path, '/') + 1;

	asprintf(&buf, FTDI_PATH "/*/%s", path);
	glob(buf, 0, NULL, &gb);
	free(buf);

	if( !gb.gl_pathc )
		return NULL;

	sscanf(*gb.gl_pathv, FTDI_PATH "/%d-%d", &bus, &device);
	globfree(&gb);

	if( (buf = get_serial(bus, device)) )
		return buf;
	return NULL;
}

#include "posix.inc"
