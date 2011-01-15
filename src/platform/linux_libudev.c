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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libudev.h>

#include "platform.h"

static char *get_monome_information(struct udev_device *d) {
	const char *serial, *tty;

	assert(d);

	if( !(tty    = udev_device_get_devnode(d)) ||
		!(serial = udev_device_get_property_value(d, "ID_SERIAL_SHORT")) )
		return NULL;

	return strdup(serial);
}

static char *get_monome_information_from_syspath(struct udev *udev,
                                                 const char *syspath) {
	struct udev_device *d = NULL;
	char *serial;

	assert(syspath);
	if( !(d = udev_device_new_from_syspath(udev, syspath)) )
		return NULL;

	serial = get_monome_information(d);
	udev_device_unref(d);

	return serial;
}

char *monome_platform_get_dev_serial(const char *device) {
	struct udev *udev;
	struct udev_enumerate *ue;
	struct udev_list_entry *c;
	const char *syspath;
	char *serial = NULL;

	assert(device);

	udev = udev_new();

	ue = udev_enumerate_new(udev);
	udev_enumerate_add_match_property(ue, "DEVNAME", device);

	if( udev_enumerate_scan_devices(ue) )
		goto err_scan;

	c = udev_enumerate_get_list_entry(ue);

	if( !(syspath = udev_list_entry_get_name(c)) )
		goto err_nodevs; /* should NOT happen ever but whatever... */

	serial = get_monome_information_from_syspath(udev, syspath);

err_nodevs:
	udev_enumerate_unref(ue);
err_scan:
	udev_unref(udev);

	return serial;
}
