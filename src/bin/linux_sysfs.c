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

#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <monome.h>

#define MAX_LENGTH 128
#define is_numeric(c) ((48 <= c) && (c <= 57))

typedef struct monome_device_mapping monome_device_mapping_t;

struct monome_device_mapping {
	char *serial;
	monome_device_t type;
	char *friendly;
};


static monome_device_mapping_t mapping[] = {
	{"m256-%d", MONOME_DEVICE_256, "monome 256"},
	{"m128-%d", MONOME_DEVICE_128, "monome 128"},
	{"m64-%d",  MONOME_DEVICE_64,  "monome 64"},
	{"m40h%d", MONOME_DEVICE_40h, "monome 40h"},
	{0, 0, 0}
};


static char *get_tty(const char *usb_device) {
	char *dirstr, *tty = NULL;
	struct dirent *ent;
	DIR *dir;
	
	if( asprintf(&dirstr, "/sys/bus/usb/devices/%s", usb_device) < 0 )
		return NULL;
	
	if( !(dir = opendir(dirstr)) )
		goto err;
	
	while( (ent = readdir(dir)) )
		if( ent->d_name[0] == 't' &&
			ent->d_name[1] == 't' &&
			ent->d_name[2] == 'y' ) {
			tty = strdup(ent->d_name);
			break;
		}
	
	closedir(dir);
	
 err:
	free(dirstr);
	return tty;
}

static char *get_serial(const char *usb_device) {
	char *filestr, buf[MAX_LENGTH], *serial;
	int len, serial_fd;
	
	len = strstr(usb_device, ":") - usb_device;
	if( !(serial = strndup(usb_device, len)) )
		return NULL;
	
	if( asprintf(&filestr, "/sys/bus/usb/devices/%s/serial", serial) < 0 ) {
		free(serial);
		return NULL;
	}
	
	free(serial);
	serial = NULL;
	
	if( (serial_fd = open(filestr, O_RDONLY)) < 0 )
		goto err;
	
	len = read(serial_fd, buf, sizeof(buf));
	if( len < 0 )
		goto file_err;
	
	serial = strndup(buf, len - 1);
	
 file_err:
	close(serial_fd);
	
 err:
	free(filestr);
	return serial;
}

static void scan_ftdi_driver_directory() {
	struct dirent *ftdi_ent;
	int i, max, serialnum;
	char *serial, *tty, *model;
	DIR *ftdi_dir;
	
	if( !(ftdi_dir = opendir("/sys/bus/usb/drivers/ftdi_sio/")) )
		return;
	
	max = sizeof(mapping) / sizeof(monome_device_mapping_t);
	serialnum = 0;
	model = NULL;

	while( (ftdi_ent = readdir(ftdi_dir)) ) {
		if( !is_numeric(*ftdi_ent->d_name) )
			continue;
		
		if( !(serial = get_serial(ftdi_ent->d_name)) )
			continue;
		
		for( i = 0; mapping[i].serial; i++ ) {
			if( !sscanf(serial, mapping[i].serial, &serialnum) )
				continue;
			
			model = mapping[i].friendly;
			break;
		}
		
		if( !model )
			goto not_monome;
		
		if( !(tty = get_tty(ftdi_ent->d_name)) )
			goto not_monome;
		
		printf("%s #%d => /dev/%s\n", model, serialnum, tty);
		
		model = NULL;
		serialnum = 0;
		free(tty);
	not_monome:
		free(serial);
	}
	
	closedir(ftdi_dir);
}

int main(int argc, char **argv) {
	scan_ftdi_driver_directory();
	
	return 0;
}
