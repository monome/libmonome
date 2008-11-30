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
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "monome.h"

#define USBFS_DEVICES "/proc/bus/usb/devices"
#define SYSFS_BUS_USB "/sys/bus/usb/devices/"

#define MAX_LENGTH 1024

struct usbdevice {
	int bus;
	int lev;
	int prnt;
	int port;
	int dev;
	
	char *manufacturer;
	char *product;
	char *serial;
};
	
static void parse_t(const char *line, struct usbdevice *d) {
	int bus, lev, prnt, port, dev;
	
	if( sscanf(line, "T: Bus=%d Lev=%d Prnt=%d Port=%d Cnt=%*d Dev#= %d Spd=%*d MxCh= %*d",
			   &bus, &lev, &prnt, &port, &dev) == 5 ) {
		d->bus  = bus;
		d->lev  = lev;
		d->prnt = prnt;
		d->port = port;
		d->dev  = dev;
	}
}

static void parse_s(const char *line, struct usbdevice *d) {
	char buf[MAX_LENGTH];
	char **dest = NULL;
	ssize_t w;
	
	line += 4;
	
	if( !strncmp(line, "Manufacturer", 12) ) {
		dest  = &d->manufacturer;
		line += 13;
	} else if( !strncmp(line, "Product", 7) ) {
		dest  = &d->product;
		line += 8;
	} else if( !strncmp(line, "SerialNumber", 12) ) {
		dest  = &d->serial;
		line += 13;
	} else
		return;
	
	for( w = 0; ( buf[w] = *line++ ); w++ ) {
		if( buf[w] == '\n' ) {
			buf[w] = '\0';
			break;
		}
	}
	
	if( buf[w] )
		buf[w--] = '\0';
	
	*dest = calloc(sizeof(char), w + 1);
	strncpy(*dest, buf, w);
}

static char *map_usb_to_device(struct usbdevice *d) {
	struct dirent *r;
	DIR *dir = NULL;
	char *u, *dev = NULL;
	
	asprintf(&u, "%s%d-%d:%d.%d", SYSFS_BUS_USB, d->bus, d->lev, d->prnt, d->port);
	if( !(dir = opendir(u)) ) {
		free(u);
		return dev;
	}
	
	while( (r = readdir(dir)) ) {
		if( strstr(r->d_name, "tty") ) {
			asprintf(&dev, "/dev/%s", r->d_name);
			break;
		}
	}
	
	free(u);
	closedir(dir);
	
	return dev;
}

static char *is_monome(struct usbdevice *d) {
	char model[3], *dev = NULL;
	
	if( d->manufacturer ) {
		if( !strcmp(d->manufacturer, "monome") ) {
			printf("%03d/%03d\n", d->bus, d->dev);
			if( !(dev = map_usb_to_device(d)) ) {
				if( strstr(d->product, "256") ) strcpy(model, "256");
				if( strstr(d->product, "128") ) strcpy(model, "128");
				if( strstr(d->product, "64" ) ) strcpy(model, "64");
				if( strstr(d->product, "40h") ) strcpy(model, "40h");
				
				printf("whoops!  I found your %s, (it's %s!), but it doesn't look like you've got the FTDI drivers installed.\n"
					   "if you like to roll your own kernels, you'll find it under:\n"
					   "    device drivers -->\n"
					   "        USB support -->\n"
					   "            USB serial converter support -->\n"
					   "                USB FTDI single port serial driver\n"
					   "                (it's CONFIG_USB_SERIAL_FTDI_SIO)\n"
					   "\n"
					   "if you don't know what that's all on about, make sure you've got the latest kernel installed from your distro's package manager.\n"
					   "if all else fails, post on the monome forums and we'll help you out.\n\n",
					   model, d->serial);
				exit(-1);
			}
		}
		
		free(d->manufacturer);
		d->manufacturer = NULL;
	}

	if( d->product ) { free(d->product); d->product = NULL; }
	if( d->serial ) { free(d->serial); d->serial = NULL; }
	
	return dev;
}

static int scan_ftdi_driver_directory() {
	DIR *dir = NULL;
	int i;
	
	if( !(dir = opendir("/sys/bus/usb/drivers/ftdi_sio/")) )
		return 0;
	
	for( i = 0; readdir(dir); i++ );
	
	i -= 5; /* ignore ".", "..", "bind", "module", and "unbind", leaving us just with the relevant USB devices */
	
	return i;
}

int monome_discover_devices(monome_t *monomes) {
	struct usbdevice d = {0, 0, 0, 0, 0, NULL, NULL, NULL};
	char line[MAX_LENGTH], *dev = NULL;
	FILE *devices;
	
	if( !(devices = fopen(USBFS_DEVICES, "r")) )
		return -1;
	
	while( fgets(line, MAX_LENGTH, devices) ) {
		switch(line[0]) {
		case 'T':
			parse_t(line, &d);
			break;
			
		case 'S':
			parse_s(line, &d);
			break;
			
		case '\n':
			if( (dev = is_monome(&d)) ) {
				printf("%s\n", dev);
				free(dev);
			}
			
			break;
		}
	}
	
	if( (dev = is_monome(&d)) ) {
		printf("%s\n", dev);
		free(dev);
	}
			
	fclose(devices);
	
	return 1;
}

int main(int argc, char **argv) {
	//monome_t *m;
	
	//monome_discover_devices(m);
	printf("%d\n", scan_ftdi_driver_directory());
	
	return 0;
}
