/*
 * This file is part of libmonome.
 * libmonome is copyright 2007, 2008 will light <visinin@gmail.com>
 *
 * libmonome is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <libudev.h>

#include <monome.h>

typedef struct monome_device_mapping monome_device_mapping_t;

struct monome_device_mapping {
	char *serial;
	monome_device_t model;
	char *friendly;
};

struct udev *udev;

static monome_device_mapping_t mapping[] = {
	{"m256-%d", MONOME_DEVICE_256, "monome 256"},
	{"m128-%d", MONOME_DEVICE_128, "monome 128"},
	{"m64-%d",  MONOME_DEVICE_64,  "monome 64"},
	{"m40h%d",  MONOME_DEVICE_40h, "monome 40h"},
	{0, 0, 0}
};

static int get_monome_information(struct udev_device *d) {
	const char *serial, *tty;
	int serialnum;

	monome_device_t model = 0;
	monome_device_mapping_t *c;

	assert(d);

	if( !(tty    = udev_device_get_devnode(d)) ||
		!(serial = udev_device_get_property_value(d, "ID_SERIAL_SHORT")) )
		return 1;

	for( c = mapping; c->serial; c++ ) {
		if( !sscanf(serial, c->serial, &serialnum) )
			continue;

		model = c->model;
		break;
	}

	if( !model )
		return 1; /* unrecognized device, disregard */

	printf("%s %s\n", tty, serial);

#if 0
	tty    = strdup(tty);
	serial = strdup(serial);
#endif

	return 0;
}

static int get_monome_information_from_syspath(const char *syspath) {
	struct udev_device *d = NULL;
	int ret;

	assert(syspath);
	if( !(d = udev_device_new_from_syspath(udev, syspath)) )
		return 1;

	ret = get_monome_information(d);
	udev_device_unref(d);

	return ret;
}

static int list_monome_devices() {
	struct udev_enumerate *ue;
	struct udev_list_entry *c;
	int found = 0;

	ue = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(ue, "tty");

	if( udev_enumerate_scan_devices(ue) ) {
		perror("udev_enumerate_scan_devices");
		goto out;
	}

	c = udev_enumerate_get_list_entry(ue);

	do {
		if( !get_monome_information_from_syspath(udev_list_entry_get_name(c)) )
			found++;
	} while( (c = udev_list_entry_get_next(c)) );

out:
	udev_enumerate_unref(ue);
	return found;
}

static void monitor_devs() {
	struct udev_monitor *m;
	struct udev_device *d;
	const char *action;

	m = udev_monitor_new_from_netlink(udev, "udev");
	udev_monitor_filter_add_match_subsystem_devtype(m, "tty", NULL);
	udev_monitor_enable_receiving(m);

	while( 1 ) {
		d = udev_monitor_receive_device(m);
		action = udev_device_get_action(d);

		switch( *action ) {
		case 'a': /* add */
			printf("=> ");
			break;

		case 'r': /* remove */
			printf("<= ");
			break;
		}

		get_monome_information(d);
	}

	udev_monitor_unref(m);
}

int main(int argc, char **argv) {
	udev = udev_new();

	if( !list_monome_devices() )
		printf("no monomes found :(\n");

	printf("\nmonitoring...\n");
	monitor_devs();

	udev_unref(udev);

	return 0;
}
