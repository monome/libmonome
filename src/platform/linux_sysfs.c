/**
 * Copyright (c) 2010 William Light <wrl@illest.net>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
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

#include "platform.h"

#define FTDI_PATH "/sys/bus/usb/drivers/ftdi_sio"
#define MAX_LENGTH 128

static char *get_serial(char *device) {
	char *filestr, buf[MAX_LENGTH], *serial;
	int len, serial_fd;

	serial = NULL;

	if( asprintf(&filestr, "/sys/bus/usb/devices/%s/serial", device) < 0 )
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
	glob_t gb;
	char *buf, *device;

	assert(path);

	if( *path == '/' )
		path++;

	path = strchr(path, '/') + 1;

	asprintf(&buf, FTDI_PATH "/*/%s", path);
	glob(buf, 0, NULL, &gb);
	free(buf);

	if( !gb.gl_pathc )
		goto err_nodevs;

	device = *gb.gl_pathv + (sizeof(FTDI_PATH) * sizeof(char));
	if( !(buf = strchr(device, ':')) )
		goto err_baddev;

	*buf = '\0';
	if( !(buf = get_serial(device)) )
		goto err_baddev;

	globfree(&gb);
	return buf;

err_baddev:
		globfree(&gb);
err_nodevs:
	return NULL;
}
