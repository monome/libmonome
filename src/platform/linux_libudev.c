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

char *
monome_platform_get_dev_serial(const char *device)
{
	struct udev_device *dev;
	struct stat statbuf;
	struct udev *udev;
	const char *serial; 
	char *ret = NULL;

	if (stat(device, &statbuf) < 0 || !S_ISCHR(statbuf.st_mode))
		goto err_stat;

	udev = udev_new();

	if (!(dev = udev_device_new_from_devnum(udev, 'c', statbuf.st_rdev)))
		goto err_no_device;

	serial = udev_device_get_property_value(dev, "ID_SERIAL_SHORT");
	if (serial) { ret = strdup(serial); }
		
	udev_device_unref(dev);
	udev_unref(udev);
	
	return ret; 

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
	const char *vendor, *model;
	
	if (stat(device, &statbuf) < 0 || !S_ISCHR(statbuf.st_mode))
		goto err_stat;

	udev = udev_new();

	if (!(dev = udev_device_new_from_devnum(udev, 'c', statbuf.st_rdev)))
		goto err_no_device;

	vendor = udev_device_get_property_value(dev, "ID_VENDOR");
	model = udev_device_get_property_value(dev, "ID_MODEL");

	printf("vendor: %s; model: %s\n\n", vendor, model);

	char res = 0;
	if (vendor != NULL && model != NULL) { 
		res = (strcmp(vendor,"monome")==0) && (strcmp(model,"grid")==0);
	} else {
		// whuh oh
		assert(0);
	}

	udev_device_unref(dev);
	udev_unref(udev);
	return res;

err_no_device:
	udev_unref(udev);
err_stat:
	return 0;
}
