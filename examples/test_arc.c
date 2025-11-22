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
 * test.c
 * basic program to test all of the output commands to the monome.
 */

#define _XOPEN_SOURCE 600

#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <curses.h>

#include <monome.h>

#define DEFAULT_MONOME_DEVICE "osc.udp://127.0.0.1:8081/monome"
#define KNOBS 4

#define BPM 98

static void chill(int speed) {
	struct timespec rem, req;

	req.tv_sec  = 0;
	req.tv_nsec = ((60000 / (BPM * speed)) * 1000000);

	nanosleep(&req, &rem);
}

void test_ring(monome_t *monome, int r) {
	uint8_t map[64] = {[0 ... 63] = 0};
	int b, i;
/*
	for( b = 0; b < 16; b++ ) {
		monome_led_ring_all(monome, r, b);
		chill(12);
	}

	for( i = -64; i < 8; i++ ) {
		monome_led_ring_set(monome, r, i & 63, (i >= 0) * 15);
		chill(40);
	}

	for( i = 1; i < 64; i++ ) {
		monome_led_ring_range(monome, r, i, i + (8 - ((i >= 55) ? i - 55: 0)), 15);
		chill(40);
		monome_led_ring_all(monome, r, 0);
	}
*/
	for( i = 0; i < 80; i++ ) {
		for( b = 0; b < 64; b++ )
			map[b] = (b - i - 1) & 0xF;

		monome_led_ring_map(monome, r, map);
		chill(24);
	}

	for( i = 0; i < 16; i++ ) {
		for( b = 0; b < 64; b++ )
			map[b] = (map[b] + 1) % (16 - i);

		monome_led_ring_map(monome, r, map);
		chill(32);
	}
}

void clear_rings(monome_t *monome) {
	int i;

	for( i = 0; i < KNOBS; i++ )
		monome_led_ring_all(monome, i, 0);
}

int main(int argc, char **argv) {
	monome_t *monome;

	if( !(monome = monome_open((argc == 2 ) ? argv[1] : DEFAULT_MONOME_DEVICE, "8000")) )
		return -1;

	uint8_t map[64];
	for(int b = 0; b < 64; b++ ) map[b] = (b - 1) & 0xF;

	clear_rings(monome);
	monome_led_ring_map(monome, 0, map);
	getchar();
	printf("intensity...\n");
	for(int i=15;i>=0;i--) {
		monome_led_ring_intensity(monome, i);
		printf("%d\n",i);
		getchar();
	}
	printf("ok\n");
	getchar();
	test_ring(monome, 0);
	getchar();
	test_ring(monome, 1);
	//monome_led_ring_map(monome, 1, map);
	getchar();
	test_ring(monome, 2);
	//monome_led_ring_map(monome, 2, map);
	getchar();
	test_ring(monome, 3);
	//monome_led_ring_map(monome, 3, map);
	return 0;
}
