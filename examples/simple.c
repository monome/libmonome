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

/**
 * simple.c
 * press a button to toggle it!
 */

#include <stdlib.h>
#include <monome.h>

uint grid[16][16];

#define MONOME_DEVICE "osc.udp://127.0.0.1:8080/monome"

/**
 * this function gets registered with monome_register_handler
 * it gets called whenever a button is pressed
 */
void handle_press(const monome_event_t *e, void *data) {
	uint x, y;

	x = e->x;
	y = e->y;

	if( grid[x][y] )
		monome_led_off(e->monome, x, y);
	else
		monome_led_on(e->monome, x, y);

	/* toggle the button */
	grid[x][y] = !grid[x][y];
}

int main(int argc, char *argv[]) {
	monome_t *monome;
	uint x, y;

	/* open the monome device */
	if( !(monome = monome_open(MONOME_DEVICE, "8000")) )
		return -1;

	monome_clear(monome, MONOME_CLEAR_OFF);

	/* initialize the grid (all off) */
	for( x = 0; x < 16; x++ )
		for( y = 0; y < 16; y++ )
			grid[x][y] = 0;

	/* register our button press callback */
	monome_register_handler(monome, MONOME_BUTTON_DOWN, handle_press, NULL);

	/* wait for presses! */
	monome_event_loop(monome);

	monome_close(monome);

	return 0;
}
