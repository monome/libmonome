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
 * life.c
 * conway's game of life
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

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

static void chill(int msec) {
	struct timespec rem, req;
	
	req.tv_nsec  = msec * 1000000;
	req.tv_sec   = req.tv_nsec / 1000000000;
	req.tv_nsec -= req.tv_sec * 1000000000;
	
	nanosleep(&req, &rem);
}

static void handle_press(const monome_event_t *e, void *user_data) {
	world[e->x][e->y].mod_next = 1;
}

static void mod_neighbors(cell_t *c, int delta) {
	int i;
	
	for( i = 0; i < 8; i++ )
		c->neighbors[i]->nnum += delta;
}

int main(int argc, char **argv) {
	monome_t *monome;
	unsigned int x, y;
	int tick = 0;
	
	cell_t *c;
	
	if( !(monome = monome_open("osc.udp://127.0.0.1:8080/life", "osc", "8000")) )
		return -1;

	monome_register_handler(monome, MONOME_BUTTON_DOWN, handle_press, NULL);
	
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
	
	monome_clear(monome, MONOME_CLEAR_OFF);

	while(1) {
		if( !(tick = !tick) )
			while( !monome_next_event(monome) );

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
}
