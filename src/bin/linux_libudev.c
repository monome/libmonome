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
#include <string.h>
#include <glob.h>
#include <errno.h>
#include <assert.h>

#include <monome.h>

#define DEV_PATH "/dev/serial/by-id/"

typedef struct monome_device_mapping monome_device_mapping_t;

struct monome_device_mapping {
	char *serial;
	monome_device_t type;
	char *friendly;
};

static monome_device_mapping_t mapping[] = {
	{"m256-%d", MONOME_DEVICE_256, "monome 256"},
	{"m128-%d", MONOME_DEVICE_128, "monome 128"},
	{"m64-%d",  MONOME_DEVICE_64,  "monome 64"},
	{"m40h%d",  MONOME_DEVICE_40h, "monome 40h"},
	{0, 0, 0}
};

void parse_device_information(char *path, char **subsystem, char **vendor, char **model, char **serial) {
	char *dev;

	/* assuming a path of the following form:
	   /dev/serial/by-id/<subsystem>-<vendor>_<model>_<serial>-<interface>-<port>

	   will look something like:
	   /dev/serial/by-id/usb-monome_monome256_m256-042-if00-port0 */

	assert(path);
	dev = path;

	while( *++dev );						/* walk to end */
	while( dev > path && *--dev != '-' );	/* disregard port */
	while( dev > path && *--dev != '-' );	/* disregard interface */
	*dev = '\0';							/* terminate string early */
	while( dev > path && *--dev != '/' );	/* find last component of path */
	dev++;

	if( subsystem )
		*subsystem = dev;

	dev = strchr(dev, '-');
	*dev++ = '\0';

	if( vendor )
		*vendor = dev;

	dev = strchr(dev, '_');
	*dev++ = '\0';

	if( model )
		*model = dev;

	dev = strchr(dev, '_');
	*dev++ = '\0';

	if( serial )
		*serial = dev;
}

int list_monome_devices() {
	char *vendor, *model, *serial;
	glob_t gb;
	int i;

	if( glob(DEV_PATH "usb-monome*", 0, NULL, &gb) ) {
		if( errno != ENOENT )
			perror("list_monome_devices");

		return 0;
	}

	for( i = 0; i < gb.gl_pathc; i++ ) {
		parse_device_information(gb.gl_pathv[i], NULL, &vendor, &model, &serial);
		printf("%s %s %s\n", vendor, model, serial);

		(void) mapping;
	}

	globfree(&gb);

	return gb.gl_pathc;
}

int main(int argc, char **argv) {
	if( !list_monome_devices() )
		printf("no monomes found :(\n");

	return 0;
}
