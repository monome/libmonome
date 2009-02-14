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
#include <unistd.h>

#include "monome.h"
#include "monome_internal.h"

int monome_device_open(monome_t *monome) {
	struct termios nt;
	
	if( (monome->fd = open(monome->dev, O_RDWR | O_NOCTTY)) < 0 ) {
		perror("libmonome: could not open monome device");
		return 1;
	}
	
	tcgetattr(monome->fd, &monome->ot);
	nt = monome->ot;
	
	cfmakeraw(&nt);
	cfsetispeed(&nt, B9600);
	cfsetospeed(&nt, B9600);
	nt.c_lflag    |=  (CLOCAL | CREAD);
	nt.c_lflag    &= ~(ECHOCTL);
	nt.c_cc[VMIN]  =  2;
	nt.c_cc[VTIME] =  5;
	
	if( tcsetattr(monome->fd, TCSANOW, &nt) < 0 ) {
		perror("libmonome: could not set terminal attributes");
		return 1;
	}
	
	return 0;
}

ssize_t monome_device_write(monome_t *monome, const uint8_t *buf, ssize_t bufsize) {
	return write(monome->fd, buf, bufsize);
}

ssize_t monome_device_read(monome_t *monome, uint8_t *buf, ssize_t count) {
	return read(monome->fd, buf, count);
}
