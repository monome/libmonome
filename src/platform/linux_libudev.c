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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <libudev.h>

#include <monome.h>
#include "internal.h"

static struct udev *udev;

static char *get_monome_information(struct udev_device *d) {
	const char *serial, *tty;

	assert(d);

	if( !(tty    = udev_device_get_devnode(d)) ||
		!(serial = udev_device_get_property_value(d, "ID_SERIAL_SHORT")) )
		return NULL;

	return strdup(serial);
}

static char *get_monome_information_from_syspath(const char *syspath) {
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
	struct udev_enumerate *ue;
	struct udev_list_entry *c;
	const char *syspath;
	char *serial = NULL;

	assert(device);

	udev = udev_new();

	ue = udev_enumerate_new(udev);
	udev_enumerate_add_match_property(ue, "DEVNAME", device);

	if( udev_enumerate_scan_devices(ue) )
		return NULL;

	c = udev_enumerate_get_list_entry(ue);

	if( !(syspath = udev_list_entry_get_name(c)) )
		goto err; /* should NOT happen ever but whatever... */

	serial = get_monome_information_from_syspath(syspath);

err:
	udev_enumerate_unref(ue);
	udev_unref(udev);
	return serial;
}

#include "posix.inc"
