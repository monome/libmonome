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

/**
 * simple.c
 * press a button to toggle it!
 *
 * this uses the raw libmonome API, not the OSC one.
 * it does not require (and does not work with) monomeserial.
 */

#include <stdlib.h>
#include <monome.h>

unsigned int grid[16][16];

/**
 * this function gets registered with monome_register_handler
 * it gets called whenever a button is pressed
 */
void handle_press(monome_event_t e, void *data) {
	if( grid[e.x][e.y] )
		monome_led_off(e.monome, e.x, e.y);
	else
		monome_led_on(e.monome, e.x, e.y);
	
	/* toggle the button */
	grid[e.x][e.y] = !grid[e.x][e.y];
}

int main(int argc, char *argv[]) {
	monome_t *monome;
	unsigned int x, y;

	/* open the monome device */
	if( !(monome = monome_open("/dev/ttyUSB0")) )
		return -1;
	
	monome_clear(monome, MONOME_CLEAR_OFF);
	
	/* initialize the grid (all off) */
	for( x = 0; x < 16; x++ )
		for( y = 0; y < 16; y++ )
			grid[x][y] = 0;
	
	/* register our button press callback */
	monome_register_handler(monome, MONOME_BUTTON_DOWN, handle_press, NULL);
	
	/* wait for presses! */
	monome_main_loop(monome);
	
	monome_close(monome);
	
	return 0;
}
