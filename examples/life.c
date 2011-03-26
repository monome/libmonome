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
 * life.c
 * conway's game of life
 */

#define _XOPEN_SOURCE 600

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <monome.h>

#define COLUMNS     16
#define ROWS        16

typedef struct cell cell_t;

struct cell {
	unsigned int alive;
	unsigned int mod_next;

	unsigned int x;
	unsigned int y;

	cell_t *neighbors[8];
	unsigned int nnum;
};

cell_t world[ROWS][COLUMNS];
monome_t *monome;

static void chill(int msec) {
	struct timespec rem, req;

	req.tv_nsec  = msec * 1000000;
	req.tv_sec   = req.tv_nsec / 1000000000;
	req.tv_nsec -= req.tv_sec * 1000000000;

	nanosleep(&req, &rem);
}

static void handle_press(const monome_event_t *e, void *user_data) {
	world[e->grid.x][e->grid.y].mod_next = 1;
}

static void mod_neighbors(cell_t *c, int delta) {
	int i;

	for( i = 0; i < 8; i++ )
		c->neighbors[i]->nnum += delta;
}

static void exit_on_signal(int s) {
	exit(EXIT_SUCCESS);
}

static void init_world() {
	unsigned int x, y;
	cell_t *c;

	for( x = 0; x < COLUMNS; x++ ) {
		for( y = 0; y < ROWS; y++ ) {
			c = &world[x][y];

			c->mod_next = 0;
			c->alive    = 0;
			c->nnum     = 0;
			c->x        = x;
			c->y        = y;

			c->neighbors[0] = &world[(x - 1) % COLUMNS][(y - 1) % ROWS];
			c->neighbors[1] = &world[(x - 1) % COLUMNS][(y + 1) % ROWS];
			c->neighbors[2] = &world[(x - 1) % COLUMNS][y];

			c->neighbors[3] = &world[(x + 1) % COLUMNS][(y - 1) % ROWS];
			c->neighbors[4] = &world[(x + 1) % COLUMNS][(y + 1) % ROWS];
			c->neighbors[5] = &world[(x + 1) % COLUMNS][y];

			c->neighbors[6] = &world[x][(y - 1) % ROWS];
			c->neighbors[7] = &world[x][(y + 1) % ROWS];
		}
	}
}

static void close_monome() {
	monome_led_all(monome, 0);
	monome_close(monome);
}

int main(int argc, char **argv) {
	unsigned int x, y;
	int tick = 0;

	cell_t *c;

	if( !(monome = monome_open("osc.udp://127.0.0.1:8080/life", "8000")) )
		return EXIT_FAILURE;

	signal(SIGINT, exit_on_signal);
	atexit(close_monome);

	monome_register_handler(monome, MONOME_BUTTON_DOWN, handle_press, NULL);

	init_world();
	monome_led_all(monome, 0);

	while(1) {
		tick++;
		if( !(tick %= 3) )
			while( monome_event_handle_next(monome) );

		for( x = 0; x < COLUMNS; x++ ) {
			for( y = 0; y < ROWS; y++ ) {
				c = &world[x][y];

				if( c->mod_next ) {
					if( c->alive ) {
						c->alive = 0;
						mod_neighbors(c, -1);

						monome_led_off(monome, x, y);
					} else {
						c->alive = 1;
						mod_neighbors(c, 1);

						monome_led_on(monome, x, y);
					}

					c->mod_next = 0;
				}
			}
		}

		for( x = 0; x < COLUMNS; x++ ) {
			for( y = 0; y < ROWS; y++ ) {
				c = &world[x][y];

				switch( c->nnum ) {
				case 3:
					if( !c->alive )
						c->mod_next = 1;

				case 2:
					break;

				default:
					if( c->alive )
						c->mod_next = 1;

					break;
				}
			}
		}

		chill(50);
	}

	return EXIT_SUCCESS;
}
