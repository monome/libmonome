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
 * simple.c
 * press a button to toggle it!
 */

#include <stdlib.h>
#include <monome.h>

unsigned int grid[16][16] = { [0 ... 15][0 ... 15] = 0 };

#define MONOME_DEVICE "osc.udp://127.0.0.1:8080/monome"

/**
 * this function gets registered with monome_register_handler
 * it gets called whenever a button is pressed
 */
void handle_press(const monome_event_t *e, void *data) {
	unsigned int x, y;

	x = e->grid.x;
	y = e->grid.y;

	/* toggle the button */
	grid[x][y] = !grid[x][y];
	monome_led_set(e->monome, x, y, grid[x][y]);
}

int main(int argc, char *argv[]) {
	monome_t *monome;

	/* open the monome device */
	if( !(monome = monome_open(MONOME_DEVICE, "8000")) )
		return -1;

	monome_led_all(monome, 0);

	/* register our button press callback */
	monome_register_handler(monome, MONOME_BUTTON_DOWN, handle_press, NULL);

	/* wait for presses! */
	monome_event_loop(monome);

	monome_close(monome);
	return 0;
}
