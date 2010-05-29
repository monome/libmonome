/*
 * This file is part of libmonome.
 * libmonome is copyright 2007-2010 will light <visinin@gmail.com>
 *
 * libmonome is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 *
 */

/**
 * test.c
 * basic program to test all of the output commands to the monome.
 */

#include <time.h>
#include <monome.h>

#define DEFAULT_MONOME_DEVICE "osc.udp://127.0.0.1:8080/monome"

#define BPM 98

uint8_t pattern[8] = { 0, 66, 102, 90, 66, 66, 66, 0 };
	
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

void test_led_row(monome_t *monome) {
	uint8_t buf_off[2], buf_on[2];
	uint i;

	buf_off[0] = buf_off[1] = 0x00;
	buf_on[0]  = buf_on[1]  = 0x05;

	for( i = 0; i < 16; i++ ) {
		monome_led_row_16(monome, i, buf_on);
		chill(16);
		monome_led_row_16(monome, i, buf_off);

		buf_on[0] <<= 1;
		buf_on[1] <<= 1;

		if( !(*buf_on & 0xFF) )
			buf_on[0]  = buf_on[1]  = 0x05;
	}
}

void test_led_col(monome_t *monome) {
	uint8_t buf_off[2], buf_on[2];
	uint i;

	buf_off[0] = buf_off[1] = 0x00;
	buf_on[0]  = buf_on[1]  = 0x05;

	for( i = 0; i < 16; i++ ) {
		monome_led_col_16(monome, i, buf_on);
		chill(16);
		monome_led_col_16(monome, i, buf_off);

		buf_on[0] <<= 1;
		buf_on[1] <<= 1;

		if( !(*buf_on & 0xFF) )
			buf_on[0]  = buf_on[1]  = 0x05;
	}
}

void test_led_frame(monome_t *monome) {
	uint i, q, l;

	for( l = 0, q = 0; l < 16; l++ ) {
		monome_led_frame(monome, q, pattern);

		for( i = 0; i < 8; i++ )
			pattern[i] ^= 0xFF;

		chill(2);

		if( l % 2 )
			q++;
	}
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
		test_led_row(monome);
		test_led_col(monome);
	}

	test_led_on_off(monome);
	test_led_frame(monome);

	fade_out(monome);

	monome_clear(monome, MONOME_CLEAR_OFF);
	monome_intensity(monome, 0x0F);

	return 0;
}
