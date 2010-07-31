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

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "monome.h"
#include "internal.h"

int monome_platform_open(monome_t *monome, const char *dev) {
	struct termios nt, ot;
	int fd;

	if( (fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0 ) {
		perror("libmonome: could not open monome device");
		return 1;
	}

	tcgetattr(fd, &ot);
	nt = ot;

	/* baud rate */
	cfsetispeed(&nt, B115200);
	cfsetospeed(&nt, B115200);

	/* parity (8N1) */
	nt.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
	nt.c_cflag |=  (CS8 | CLOCAL | CREAD);

	/* no line processing */
	nt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG | IEXTEN);

	/* raw input */
	nt.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK |
	                INPCK | ISTRIP | IXON);

	/* raw output */
	nt.c_oflag &= ~(OCRNL | ONLCR | ONLRET | ONOCR |
	                OFILL | OPOST);

	/* block until one character is read */
	nt.c_cc[VMIN]  = 1;
	nt.c_cc[VTIME] = 0;

	if( tcsetattr(fd, TCSANOW, &nt) < 0 ) {
		perror("libmonome: could not set terminal attributes");
		return 1;
	}

	tcflush(fd, TCIOFLUSH);

	monome->fd = fd;
	monome->ot = ot;

	return 0;
}

int monome_platform_close(monome_t *monome) {
	if( tcsetattr(monome->fd, TCSANOW, &monome->ot) < 0 )
		perror("libmonome: could not restore terminal attributes");

	return close(monome->fd);
}

ssize_t monome_platform_write(monome_t *monome, const uint8_t *buf, ssize_t bufsize) {
	int ret;
	fd_set fds;

	FD_ZERO(&fds);
	FD_SET(monome->fd, &fds);

	if( select(monome->fd + 1, NULL, &fds, NULL, NULL) < 0 ) {
		perror("libmonome: error in select()");
		return -1;
	}

	ret = write(monome->fd, buf, bufsize);
	tcdrain(monome->fd);

	return ret;
}

ssize_t monome_platform_read(monome_t *monome, uint8_t *buf, ssize_t count) {
	return read(monome->fd, buf, count);
}
