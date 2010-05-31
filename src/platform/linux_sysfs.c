/*
 * Copyright (c) 2007-2010, William Light <will@visinin.com>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * 
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 * 
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
#include "internal.h"

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
