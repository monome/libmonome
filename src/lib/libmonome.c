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

#include "monome.h"
#include "monome_internal.h"
#include "monome_platform.h"
#include "monome_protocol.h"

/**
 * private
 */

static ssize_t monome_read(monome_t *monome, uint8_t *buf, ssize_t count) {
    return monome_device_read(monome, buf, count);
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
	
	if( event_type > 1 || !cb )
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
	
	if( ((event_type > 1 ) | !(handler_curs = monome->handlers[event_type])) || !cb )
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
	monome_event_t e;
	uint8_t buf[2] = {0, 0};
	
	e.monome = monome;
	
	while( monome_read(monome, buf, sizeof(buf)) > 0 ) {
		monome_protocol_populate_event(&e, buf, sizeof(buf));
		
		for( handler_curs = monome->handlers[e.event_type] ; handler_curs ; handler_curs = handler_curs->next )
			if( handler_curs->cb )
				handler_curs->cb(e, handler_curs->data);
			
		buf[0] = buf[1] = 0;
	}
}
