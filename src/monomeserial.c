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
#include <string.h>
#include <stdlib.h>

#include <lo/lo.h>

#include "monome.h"

#define DEFAULT_MONOME_DEVICE 	"/dev/ttyUSB0"
#define DEFAULT_OSC_PREFIX		"monome"
#define DEFAULT_OSC_HOST_PORT	"8000"
#define DEFAULT_OSC_LISTEN_PORT	"8080"

char *lo_prefix;
lo_address *outgoing;
lo_server_thread *st;

void add_osc_methods(char*, monome_t*);
void lo_error(int, const char*, const char*);
void handle_press(monome_event_t, void*);
int clear_handler(const char*, const char*, lo_arg**, int, lo_message, void*);
int led_handler(const char*, const char*, lo_arg**, int, lo_message, void*);
int led_col_row_handler(const char*, const char*, lo_arg**, int, lo_message, void*);
int frame_handler(const char*, const char*, lo_arg**, int, lo_message, void*);

int main(int argc, char *argv[]) {
	monome_t *monome;
	
	if( argc == 2 )
		lo_prefix = argv[1];
	else {
		lo_prefix = calloc(sizeof(char), strlen(DEFAULT_OSC_PREFIX) + 1);
		strcpy(lo_prefix, DEFAULT_OSC_PREFIX);
	}
	
	if( !(monome = monome_open((argc == 3 ) ? argv[2] : DEFAULT_MONOME_DEVICE)) )
		return -1;
	
	if( !(st = lo_server_thread_new(DEFAULT_OSC_LISTEN_PORT, lo_error)) )
		return -1;
	
	outgoing = lo_address_new(NULL, DEFAULT_OSC_HOST_PORT);
	
	monome_register_handler(monome, MONOME_BUTTON_DOWN, handle_press, lo_prefix);
	monome_register_handler(monome, MONOME_BUTTON_UP, handle_press, lo_prefix);
	
	add_osc_methods(lo_prefix, monome);
	
	monome_clear(monome, MONOME_CLEAR_OFF);
	
	lo_server_thread_start(st);
	monome_main_loop(monome);
	
	monome_close(monome);
	free(lo_prefix);
	
	return 0;
}

void add_osc_methods(char *prefix, monome_t *monome) {
	char *cmd_buf;
	
	asprintf(&cmd_buf, "/%s/clear", prefix);
	lo_server_thread_add_method(st, cmd_buf, "", clear_handler, monome);
	lo_server_thread_add_method(st, cmd_buf, "i", clear_handler, monome);
	free(cmd_buf);
	
	asprintf(&cmd_buf, "/%s/led", prefix);
	lo_server_thread_add_method(st, cmd_buf, "iii", led_handler, monome);
	free(cmd_buf);
	
	asprintf(&cmd_buf, "/%s/led_row", prefix);
	lo_server_thread_add_method(st, cmd_buf, "ii", led_col_row_handler, monome);
	lo_server_thread_add_method(st, cmd_buf, "iii", led_col_row_handler, monome);
	free(cmd_buf);
	
	asprintf(&cmd_buf, "/%s/led_col", prefix);
	lo_server_thread_add_method(st, cmd_buf, "ii", led_col_row_handler, monome);
	lo_server_thread_add_method(st, cmd_buf, "iii", led_col_row_handler, monome);
	free(cmd_buf);
	
	asprintf(&cmd_buf, "/%s/frame", prefix);
	lo_server_thread_add_method(st, cmd_buf, "iiiiiiii", frame_handler, monome);
	lo_server_thread_add_method(st, cmd_buf, "iiiiiiiii", frame_handler, monome);
	lo_server_thread_add_method(st, cmd_buf, "iiiiiiiiii", frame_handler, monome);
	free(cmd_buf);
}

void del_osc_methods(char *prefix) {
	char *cmd_buf;
	
	asprintf(&cmd_buf, "/%s/clear", prefix);
	lo_server_thread_del_method(st, cmd_buf, "");
	lo_server_thread_del_method(st, cmd_buf, "i");
	free(cmd_buf);
	
	asprintf(&cmd_buf, "/%s/led", prefix);
	lo_server_thread_del_method(st, cmd_buf, "iii");
	free(cmd_buf);
	
	asprintf(&cmd_buf, "/%s/led_row", prefix);
	lo_server_thread_del_method(st, cmd_buf, "ii");
	lo_server_thread_del_method(st, cmd_buf, "iii");
	free(cmd_buf);

	asprintf(&cmd_buf, "/%s/led_col", prefix);
	lo_server_thread_del_method(st, cmd_buf, "ii");
	lo_server_thread_del_method(st, cmd_buf, "iii");
	free(cmd_buf);

	asprintf(&cmd_buf, "/%s/frame", prefix);
	lo_server_thread_del_method(st, cmd_buf, "iiiiiiii");
	lo_server_thread_del_method(st, cmd_buf, "iiiiiiiii");
	lo_server_thread_del_method(st, cmd_buf, "iiiiiiiiii");
	free(cmd_buf);
}

void lo_error(int num, const char *error_msg, const char *path) {
	printf("monomeserial: lo server error %d in %s: %s\n", num, path, error_msg);
	fflush(stdout);
}

int clear_handler(const char *path, const char *types, lo_arg **argv, int argc, lo_message data, void *user_data) {
	monome_t *monome = user_data;
	int mode = 0;

	if( argc > 0 )
		mode = argv[0]->i & 0x01;
	
	return monome_clear(monome, mode);
}

int led_handler(const char *path, const char *types, lo_arg **argv, int argc, lo_message data, void *user_data) {
	monome_t *monome = user_data;
	
	if( ( argc != 3  || strcmp("iii", types) ) ||
		( argv[0]->i > 15 || argv[0]->i < 0 )  ||
		( argv[1]->i > 15 || argv[1]->i < 0 )  ||
		( argv[2]->i > 1  || argv[2]->i < 0 )  )
		return -1;
	
	if( argv[2]->i )
		return monome_led_on(monome, argv[0]->i, argv[1]->i);
	else
		return monome_led_off(monome, argv[0]->i, argv[1]->i);
}

int led_col_row_handler(const char *path, const char *types, lo_arg **argv, int argc, lo_message data, void *user_data) {
	monome_t *monome = user_data;
	uint8_t buf[2];
	
	if( argv[0]->i > 15 || argv[0]->i < 0 )
		return -1;
	
	switch( argc ) {
	case 2:
		if( strncmp("ii", types, 2) )
			return -1;
		
		buf[0] = argv[1]->i;
		
		if( strstr(path, "led_col") )
			return monome_led_col_8(monome, argv[0]->i, buf);
		else
			return monome_led_row_8(monome, argv[0]->i, buf);
		
		break;
		
	case 3:
		if( strncmp("iii", types, 3) )
			return -1;

		buf[0] = argv[1]->i;
		buf[1] = argv[2]->i;
		
		if( strstr(path, "led_col") )
			return monome_led_col_16(monome, argv[0]->i, buf);
		else
			return monome_led_row_16(monome, argv[0]->i, buf);
		
		break;
	}
	
	return 0;
}

int frame_handler(const char *path, const char *types, lo_arg **argv, int argc, lo_message data, void *user_data) {
	monome_t *monome = user_data;
	uint8_t buf[8], i;

	for( i = 0; i < 8; i++ )
		buf[i] = argv[i]->i;
	
	switch( argc ) {
	case 8:
		return monome_led_frame(monome, 0, buf);
		break;
		
	case 9:
		return monome_led_frame(monome, argv[8]->i, buf);
		break;
		
	case 10:
		/**
		 * okay, this isn't implemented yet.
		 * passing 10 arguments to /frame means you want to offset it by argv[8] and argv[9]
		 * thing is, there's no clean mapping to the serial protocol
		 * so this is going to have to wait until v0.2 at the earliest.
		 */
		break;
	}
	
	return -1;
}

void handle_press(monome_event_t e, void *data) {
	char *cmd;
	char *prefix = data;

	asprintf(&cmd, "/%s/press", prefix);
	lo_send_from(outgoing, lo_server_thread_get_server(st), LO_TT_IMMEDIATE, cmd, "iii", e.x, e.y, !(e.event_type >> 4));
	free(cmd);
}
