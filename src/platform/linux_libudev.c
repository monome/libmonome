/**
 * Copyright (c) 2013 Nedko Arnaudov <nedko@arnaudov.name>
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
#include <errno.h>

#include "platform.h"

int monome_platform_get_udev_info(const char * device, char ** serial_ptr_ptr, char ** devnode_ptr_ptr, monome_error_t error)
{
	int ret;
	struct stat statbuf;
	struct udev * udev;
	const char * serial;
	const char * devnode;
	struct udev_device * udev_device;

	assert(device != NULL);
	assert(serial_ptr_ptr != NULL || devnode_ptr_ptr != NULL);

	monome_debug("device is \"%s\"", device);

	udev = udev_new();
	if (udev == NULL) {
		monome_error_set(error, "Creation of udev library context failed");
		ret = -1;
		goto exit;
	}

	if (stat(device, &statbuf) != 0) {
		if (device[0] != 0 && errno == ENOENT) goto device_id;
		ret = errno;
		monome_error_set(error, "Cannot check \"%s\" file type. %s", device, strerror(errno));
		goto unref_udev;
	}

	if (S_ISCHR(statbuf.st_mode)) {
		monome_debug("character device %d:%d", major(statbuf.st_rdev), minor(statbuf.st_rdev));

		udev_device = udev_device_new_from_devnum(udev, 'c', statbuf.st_rdev);
		if (udev_device == NULL) {
			monome_error_set(error, "cannot open udev by syspath \"%s\"", device);
			ret = -1;
			goto unref_udev;
		}
	} else if (device[0] == '/') {
		monome_debug("starts with slash, assuming syspath");
		udev_device = udev_device_new_from_syspath(udev, device);
		if (udev_device == NULL) {
			monome_error_set(error, "cannot open udev by syspath \"%s\"", device);
			ret = -1;
			goto unref_udev;
		}
	} else {
	device_id:
		monome_debug("assuming libudev device id format");
		udev_device = udev_device_new_from_device_id(udev, (char *)device);
		//monome_debug("path=\"%s\"", device); // why udev_device_new_from_device_id() wants non-const string?
		if (udev_device == NULL) {
			monome_error_set(error, "cannot open udev by device id \"%s\"", device);
			ret = -1;
			goto unref_udev;
		}
	}

	devnode = udev_device_get_devnode(udev_device);
	if (devnode == NULL) {
		monome_error_set(error, "device without devnode");
		ret = -1;
		goto unref_udev_device;
	}

	monome_debug("devnode is \"%s\"", devnode);

	serial = udev_device_get_property_value(udev_device, "ID_SERIAL_SHORT");
	if (serial == NULL) {
		monome_error_set(error, "device without ID_SERIAL_SHORT property");
		ret = -1;
		goto unref_udev_device;
	}

	monome_debug("serial is \"%s\"", serial);

	if (serial_ptr_ptr != NULL) {
		*serial_ptr_ptr = strdup(serial);
		if (*serial_ptr_ptr == NULL) {
			monome_error_set(error, "Out of memory (strdup(serial))");
			goto unref_udev_device;
		}
	}

	if (devnode_ptr_ptr != NULL) {
		*devnode_ptr_ptr = strdup(devnode);
		if (*devnode_ptr_ptr == NULL) {
			monome_error_set(error, "Out of memory (strdup(devnode))");
			goto free_serial;
		}
	}

	ret = 0;
	goto unref_udev_device;

free_serial:
	if (serial_ptr_ptr != NULL) free(*serial_ptr_ptr);
unref_udev_device:
	udev_device_unref(udev_device);
unref_udev:
	udev_unref(udev);
exit:
	return ret;
}

char * monome_platform_get_dev_serial(const char * device)
{
	int ret;
	monome_error_t err;
	char * serial;

	monome_error_init(&err);

	ret = monome_platform_get_udev_info(device, &serial, NULL, err);
	if (ret != 0) {
		fprintf(stderr, "%s (%d)\n", monome_error_get(err), ret);
		monome_error_uninit(err);
		return NULL;
	}

	monome_error_uninit(err);

	return serial;
}
