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
