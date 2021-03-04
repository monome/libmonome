/**
 * Copyright (c) 2015 William Light <wrl@illest.net>
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
#include <sys/stat.h>

#include <libudev.h>

#include "platform.h"

static char *
get_monome_information(struct udev_device *d)
{
	const char *serial;

	serial = udev_device_get_property_value(d, "ID_SERIAL_SHORT");

	return (serial) ? strdup(serial) : NULL;
}

char *
monome_platform_get_dev_serial(const char *device)
{
	struct udev_device *dev;
	struct stat statbuf;
	struct udev *udev;
	char *serial;

	if (stat(device, &statbuf) < 0 || !S_ISCHR(statbuf.st_mode))
		goto err_stat;

	udev = udev_new();

	if (!(dev = udev_device_new_from_devnum(udev, 'c', statbuf.st_rdev)))
		goto err_no_device;

	serial = get_monome_information(dev);

	udev_device_unref(dev);
	udev_unref(udev);

	return serial;

err_no_device:
	udev_unref(udev);
err_stat:
	return NULL;
}

char
monome_platform_is_dev_grid(const char *device)
{
	struct udev_device *dev;
	struct stat statbuf;
	struct udev *udev;
	
	if (stat(device, &statbuf) < 0 || !S_ISCHR(statbuf.st_mode))
		goto err_stat;

	udev = udev_new();

	if (!(dev = udev_device_new_from_devnum(udev, 'c', statbuf.st_rdev)))
		goto err_no_device;

	const char *vendor = udev_device_get_property_value(udev, "ID_VENDOR");
	const char *model = udev_device_get_property_value(udev, "ID_MODEL");
	char res = (strcmp(vendor,"monome")==0) && (strcmp(model,"grid")==0);
	
	udev_device_unref(dev);
	udev_unref(udev);
	return res;

err_no_device:
	udev_unref(udev);
err_stat:
	return 0;
}
