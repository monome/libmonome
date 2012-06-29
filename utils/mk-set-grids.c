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

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include <monome.h>
#include "internal.h"
#include "platform.h"
#include "devices.h"

/*
 * mk-set-grids.c:
 *  for setting the active number of grids in the new-school monome kit (mk)
 *  devices
 *
 * uses the internal raw serial writing API, not a good idea to use in your
 * own code!
 */

#define PROTO_MK_GRIDS 12

static void usage(const char *app) {
	printf("usage: %s -d <device> -g <active grids>\n"
		   "\n"
		   "  -h, --help			display this information\n"
		   "\n"
		   "  -d, --device <device>		the monome serial device\n"
		   "  -g, --active-grids <n>	how many grids are connected to the mk device\n"
		   "\n", app);
}

int main(int argc, char **argv) {
	char c, *device;
	uint8_t buf[1];
	int grids, i;

	monome_t mk;

	struct option arguments[] = {
		{"help",         no_argument,       0, 'h'},
		{"device",       required_argument, 0, 'd'},
		{"active-grids", required_argument, 0, 'g'}
	};

	grids = 0;
	device = NULL;
	i = 0;

	while( (c = getopt_long(argc, argv, "hd:g:", arguments, &i)) > 0 )
		switch( c ) {
		case 'h':
			usage(argv[0]);
			return 1;

		case 'd':
			device = optarg;
			break;

		case 'g':
			grids = atoi(optarg);
			break;
		}

	if( !device || !grids ) {
		usage(argv[0]);
		exit(EXIT_FAILURE);
	}

	memset(&mk, '\0', sizeof(monome_t));

	/* monome_platform_open() needs a devmap to check if it has any quirks.
	   &mapping[3] is the device entry for the mk. */
	if( monome_platform_open(&mk, &mapping[3], device) )
		exit(EXIT_FAILURE);

	buf[0] = (PROTO_MK_GRIDS << 4) | (grids & 0xF);
	monome_platform_write(&mk, buf, 1);

	printf("successfully set active grids on %s\n", device);

	monome_platform_close(&mk);
	return EXIT_SUCCESS;
}
