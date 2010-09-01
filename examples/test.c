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

#include <time.h>
#include <monome.h>

#define DEFAULT_MONOME_DEVICE "osc.udp://127.0.0.1:8080/monome"

#define BPM 98

uint8_t pattern[4][8] = {
	/* Y */ {0, 34, 20, 8, 8, 8, 8, 0},
	/* E */ {0, 126, 2, 2, 30, 2, 126},
	/* A */ {0, 124, 66, 66, 126, 66, 66, 0},
	/* H */ {0, 36, 36, 36, 60, 36, 36, 0}
};

static void chill(int speed) {
	struct timespec rem, req = {0, ((60000 / (BPM * speed)) * 1000000)};
	nanosleep(&req, &rem);
}

void test_led_on_off(monome_t *monome) {
	uint i, j;

	for( i = 0; i < 16; i++ )
		for( j = 0; j < 16; j++ ) {
			monome_led_on(monome, j, i);
			chill(128);
		}

	for( i = 0; i < 16; i++ )
		for( j = 0; j < 16; j++ ) {
			monome_led_off(monome, j, i);
			chill(128);
		}
}

void test_led_row_8(monome_t *monome, uint8_t on) {
	uint i;

	for( i = 0; i < 8; i++ ) {
		monome_led_row(monome, i, 1, (uint8_t *) &on);
		chill(16);

		on |= on << 1;
	}

	for( ; i < 16; i++ ) {
		monome_led_row(monome, i, 1, (uint8_t *) &on);
		chill(16);

		on >>= 1;
	}
}

void test_led_col_8(monome_t *monome, uint8_t on) {
	uint i;

	for( i = 0; i < 8; i++ ) {
		monome_led_col(monome, i, 1, (uint8_t *) &on);
		chill(16);

		on |= on << 1;
	}

	for( ; i < 16; i++ ) {
		monome_led_col(monome, i, 1, (uint8_t *) &on);
		chill(16);

		on >>= 1;
	}
}

void test_led_row_16(monome_t *monome, uint16_t on) {
	uint i;

	for( i = 0; i < 16; i++ ) {
		monome_led_row(monome, i, 2, (uint8_t *) &on);
		chill(16);

		on |= on << 1;
	}
}

void test_led_col_16(monome_t *monome, uint16_t on) {
	uint i;

	for( i = 0; i < 16; i++ ) {
		monome_led_col(monome, i, 2, (uint8_t *) &on);
		chill(16);

		on |= on << 1;
	}
}

void test_led_frame(monome_t *monome) {
	uint i, q, l;

	for( l = 0, q = 0; l < 8; l++ ) {
		monome_led_frame(monome, q, pattern[q]);

		for( i = 0; i < 8; i++ )
			pattern[q][i] ^= 0xFF;

		chill(2);

		if( l % 2 )
			q = (q + 1) & 3;
	}
}

void test_mode(monome_t *monome) {
	monome_mode(monome, MONOME_MODE_SHUTDOWN);
	chill(2);
	monome_mode(monome, MONOME_MODE_TEST);
	chill(2);
	monome_mode(monome, MONOME_MODE_NORMAL);
	chill(1);
}

void fade_out(monome_t *monome) {
	uint i = 0x10;

	while( i-- ) {
		monome_intensity(monome, i);
		chill(16);
	}
}

int main(int argc, char **argv) {
	monome_t *monome;
	int i;
	i =0;

	if( !(monome = monome_open((argc == 2 ) ? argv[1] : DEFAULT_MONOME_DEVICE, "8000")) )
		return -1;

	monome_clear(monome, MONOME_CLEAR_OFF);

	for( i = 0; i < 2; i++ ) {
		test_led_row_8(monome, 1);
		test_led_col_8(monome, 1);
	}

	for( i = 0; i < 2; i++ ) {
		test_led_row_16(monome, 1);
		test_led_col_16(monome, 1);
	}

	test_led_col_16(monome, 0);
	test_led_on_off(monome);
	test_led_frame(monome);

	chill(4);
	test_mode(monome);

	fade_out(monome);

	monome_clear(monome, MONOME_CLEAR_OFF);
	monome_intensity(monome, 0x0F);

	return 0;
}
