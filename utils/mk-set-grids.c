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

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include <monome.h>
#include "internal.h"
#include "platform.h"

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
	uint8_t buf[2];
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
	if( monome_platform_open(&mk, device) )
		exit(EXIT_FAILURE);

	buf[0] = (PROTO_MK_GRIDS << 4) | (grids & 0xF);
	monome_platform_write(&mk, buf, 1);

	monome_platform_close(&mk);
	return EXIT_SUCCESS;
}
