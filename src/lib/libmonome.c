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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "monome_internal.h"
#include "monome_platform.h"

/**
 * private
 */

static ssize_t monome_write(monome_t *monome, const uint8_t *buf, ssize_t bufsize) {
	return monome_device_write(monome, buf, bufsize);
}

static ssize_t monome_read(monome_t *monome, uint8_t *buf, ssize_t count) {
	return monome_device_read(monome, buf, count);
}

static ssize_t monome_led_col_row(monome_t *monome, unsigned int mode, unsigned int address, unsigned int *data) {
	uint8_t buf[3];
	
	switch( mode ) {
	case MONOME_LED_COL_8:
	case MONOME_LED_ROW_8:
		buf[0] = mode | (address & 0x0F );
		buf[1] = data[0];
		
		return monome_write(monome, buf, sizeof(buf) - sizeof(char));
		
	case MONOME_LED_COL_16:
	case MONOME_LED_ROW_16:
		buf[0] = mode | (address & 0x0F );
		buf[1] = data[0];
		buf[2] = data[1];
		
		return monome_write(monome, buf, sizeof(buf));
	}
	
	return -1;
}

static ssize_t monome_led(monome_t *monome, unsigned int status, unsigned int x, unsigned int y) {
	uint8_t buf[2];
	
	buf[0] = status;
	buf[1] = ( x << 4 ) | y;
	
	return monome_write(monome, buf, sizeof(buf));
}

/**
 * public
 */

monome_t *monome_init() {
	monome_t *monome = malloc(sizeof(monome_t));

	monome->handlers[0]       = malloc(sizeof(monome_callback_t));
	monome->handlers[0]->cb   = NULL;
	monome->handlers[0]->data = NULL;
	monome->handlers[0]->next = NULL;

	monome->handlers[1]       = malloc(sizeof(monome_callback_t));
	monome->handlers[1]->cb   = NULL;
	monome->handlers[1]->data = NULL;
	monome->handlers[1]->next = NULL;
	
	return monome;
}

monome_t *monome_open(const char *dev) {
	monome_t *monome = monome_init();
	monome->dev = strdup(dev);
	
	if( monome_device_open(monome) )
		return NULL;
	return monome;
}

void monome_close(monome_t *monome) {
	if( tcsetattr(monome->fd, TCSANOW, &monome->ot) < 0) {
		perror("libmonome: could not restore terminal attributes");
		return;
	}
	
	close(monome->fd);
	
	free(monome->dev);
	free(monome);
}

void monome_register_handler(monome_t *monome, unsigned int event_type, monome_callback_function_t cb, void *data) {
	monome_callback_t *handler, *handler_curs;
	
	if( (event_type >>= 4) > 1 || !cb )
		return;
	
	handler = malloc(sizeof(monome_callback_t));
	
	handler->cb   = cb;
	handler->data = data;
	handler->next = NULL;
	
	for( handler_curs = monome->handlers[event_type]; handler_curs->next; handler_curs = handler_curs->next );
	handler_curs->next = handler;
}

/**
 * if NULL is passed instead of the final argument, all instances of cb will be removed from the callback list.
 *
 * FIXME: if a function is registered as a callback with NULL passed as the user_data argument, it can't be removed without getting rid of all of the other instances of the function.
 */

void monome_unregister_handler(monome_t *monome, unsigned int event_type, monome_callback_function_t cb, void *data) {
	monome_callback_t *handler_curs, *handler_next;
	
	if( (((event_type >>= 4) > 1 ) | !(handler_curs = monome->handlers[event_type])) || !cb )
		return;
	
	do {
		if( handler_curs->next ) {
			handler_next = handler_curs->next;
			
			if( handler_next->cb == cb && ( handler_next->data == data || !data ) ) {
				if( monome->handlers[event_type] == handler_next )
					monome->handlers[event_type]  = handler_next->next;
				
				handler_curs->next = handler_next->next;
				free(handler_next);
				
				continue;
			}
		}
		
	} while ( (handler_curs = handler_curs->next) );
}

void monome_main_loop(monome_t *monome) {
	monome_callback_t *handler_curs;
	unsigned int event_shifted;
	monome_event_t e;
	uint8_t buf[2];
	
	e.monome = monome;
	
	while( monome_read(monome, buf, sizeof(buf)) > 0 ) {
		switch( (e.event_type = buf[0]) ) {
		case MONOME_BUTTON_DOWN:
		case MONOME_BUTTON_UP:
			event_shifted = e.event_type >> 4;
			e.x = buf[1] >> 4;
			e.y = buf[1] & 0x0F;
			
			for( handler_curs = monome->handlers[event_shifted] ; handler_curs ; handler_curs = handler_curs->next )
				if( handler_curs->cb )
					handler_curs->cb(e, handler_curs->data);
			
			break;
		
			/** leaving room here for ADC **/
		}
	}
}

ssize_t monome_clear(monome_t *monome, monome_clear_status_t status) {
	uint8_t buf = MONOME_CLEAR | ( status & MONOME_CLEAR_ON );
	return monome_write(monome, &buf, sizeof(buf));
}

ssize_t monome_intensity(monome_t *monome, unsigned int brightness) {
	uint8_t buf = MONOME_INTENSITY | ( brightness & 0x0F );
	return monome_write(monome, &buf, sizeof(buf));
}

ssize_t monome_mode(monome_t *monome, monome_mode_t mode) {
	uint8_t buf = MONOME_MODE | ( (mode & MONOME_MODE_TEST) | (mode & MONOME_MODE_SHUTDOWN) );
	return monome_write(monome, &buf, sizeof(buf));
}

ssize_t monome_led_on(monome_t *monome, unsigned int x, unsigned int y) {
	return monome_led(monome, MONOME_LED_ON, x, y);
}

ssize_t monome_led_off(monome_t *monome, unsigned int x, unsigned int y) {
	return monome_led(monome, MONOME_LED_OFF, x, y);
}

ssize_t monome_led_col_8(monome_t *monome, unsigned int col, unsigned int *col_data) {
	return monome_led_col_row(monome, MONOME_LED_COL_8, col, col_data);
}

ssize_t monome_led_row_8(monome_t *monome, unsigned int row, unsigned int *row_data) {
	return monome_led_col_row(monome, MONOME_LED_ROW_8, row, row_data);
}

ssize_t monome_led_col_16(monome_t *monome, unsigned int col, unsigned int *col_data) {
	return monome_led_col_row(monome, MONOME_LED_COL_16, col, col_data);
}

ssize_t monome_led_row_16(monome_t *monome, unsigned int row, unsigned int *row_data) {
	return monome_led_col_row(monome, MONOME_LED_ROW_16, row, row_data);
}

ssize_t monome_led_frame(monome_t *monome, unsigned int quadrant, unsigned int *frame_data) {
	uint8_t buf[9], i;
	
	buf[0] = MONOME_LED_FRAME | ( quadrant & 0x03 );
	
	for( i = 1; i < 9; i++ )
		if( !(buf[i] = *(frame_data++)) )
			return 0;
	
	return monome_write(monome, buf, sizeof(buf));
}
