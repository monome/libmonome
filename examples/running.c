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

#define _BSD_SOURCE             /* usleep() */
#include <unistd.h>             /* usleep() */
#include <stdlib.h>             /* EXIT_FAILURE */
#include <string.h>             /* memset() */

#include <monome.h>

#define DEFAULT_MONOME_DEVICE   "/dev/ttyUSB0"

int main(int argc, char **argv)
{
	monome_t * monome;
	unsigned int index;
	unsigned int cols;
	unsigned int rows;
	uint8_t map[8];
	if (!(monome = monome_open((argc == 2 ) ? argv[1] : DEFAULT_MONOME_DEVICE)))
		return EXIT_FAILURE;

	rows = monome_get_rows(monome);
	cols = monome_get_cols(monome);

	memset(map, 0, sizeof(map));
	index = 0;
	while (1) {
		if (index >= 0) {
			monome_led_set(monome, index % cols, index / cols, 0);
		}
    
		index = (index + 1) % (rows * cols);
		//monome_led_map(monome, 0, 0, map);
		monome_led_set(monome, index % cols, index / cols, 1);
		usleep(100 * 1000);         /* 100ms */
	}
}
