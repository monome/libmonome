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

#include <stdio.h>
#include "monome.h"

void handle_press(monome_event_t e, void *data) {
	uint8_t (*grid)[16] = data;
	
	if ( !(grid[e.x][e.y] = !grid[e.x][e.y]) )
		monome_led_off(e.monome, e.x, e.y);
	else
		monome_led_on(e.monome, e.x, e.y);
}

int main(int argc, char *argv[]) {
	monome_t *monome;
	uint8_t x, y, grid[16][16];

	if( !(monome= monome_open("/dev/ttyUSB0")) )
		return -1;
	
	monome_clear(monome, MONOME_CLEAR_OFF);
	
	for( x = 0; x < 16; x++ )
		for( y = 0; y < 16; y++ )
			grid[x][y] = 0;
	
	monome_register_handler(monome, MONOME_BUTTON_DOWN, handle_press, grid);
	
	monome_main_loop(monome);
	
	monome_close(monome);
	
	return 0;
}
