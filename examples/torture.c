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

/**
 * torture.c
 * write a ton of data to a monome
 */

#define _XOPEN_SOURCE 600

#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include <monome.h>

#define MONOME_OSC "osc.udp://127.0.0.1:8080/monome"
#define MONOME_SERIAL "/dev/ttyUSB0"

#define WIDTH  16
#define HEIGHT 16

void random_chill() {
	struct timespec rem, req;

	req.tv_sec = 0;
	req.tv_nsec = (random() % 100000) + 100;

	nanosleep(&req, &rem);
}

int main(int argc, char **argv) {
	monome_t *monome;
	unsigned int w, h, y, s;
	uint16_t buf;

	if( !(monome = monome_open(MONOME_OSC, "8000")) ) {
		fprintf(stderr, "couldn't open monome\n");
		exit(EXIT_FAILURE);
	}

	w = WIDTH;
	h = HEIGHT;

	for(s = 0;; s = !s)
		for( y = 0; y < h; y++ ) {
			buf = ((1 << y)) - s;
			monome_led_row(monome, y, w / 8, y, (uint8_t *) &buf);
			monome_led_set(monome, w - 1, y, random() & 1);
			random_chill();
		}
}
