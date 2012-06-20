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

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>

#include "platform.h"

char *monome_platform_get_dev_serial(const char *path) {
	char *serial;

	assert(path);

	/* osx serial paths are of the form
	   /dev/tty.usbserial-<device serial> or
	   /dev/tty.usbmodem<device serial> (arduino uno)

	   we'll locate to one past the first hyphen
	   or the first occurrence of usbmodem sequence */

	if( (serial = strstr(path, "usbmodem")) )
		serial += 7;
	else if( !(serial = strchr(path, '-')) )
		return NULL;

	return strdup(serial + 1);
}

int monome_platform_wait_for_input(monome_t *monome, uint_t msec) {
	struct timeval timeout[1];
	fd_set rfds[1];
	int fd;

	fd = monome_get_fd(monome);

	timeout->tv_sec  = msec / 1000;
	timeout->tv_usec = (msec - (timeout->tv_sec * 1000)) * 1000;

	FD_ZERO(rfds);
	FD_SET(fd, rfds);

	if( !select(fd + 1, rfds, NULL, NULL, timeout) )
		return 1;

	return 0;
}
