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

/* for asprintf */
#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/select.h>
#include <termios.h>
#include <errno.h>

#include <monome.h>
#include "internal.h"
#include "platform.h"

#define MONOME_BAUD_RATE B115200
#define READ_TIMEOUT 25

#if !defined(EMBED_PROTOS)
/* stops gcc from complaining when compiled with -pedantic */
typedef union {
	void *vptr;
	monome_t *(*func)();
} func_vptr_t;

monome_t *monome_platform_load_protocol(const char *proto) {
	void *dl_handle;
	func_vptr_t protocol_new;
	monome_t *monome;
	char *buf;

	if( asprintf(&buf, "%s/monome/protocol_%s%s", LIBDIR, proto, LIBSUFFIX) < 0 )
		return NULL;

	dl_handle = dlopen(buf, RTLD_LAZY);
	free(buf);

	if( !dl_handle ) {
		fprintf(stderr, "couldn't load monome protocol module.  "
				"dlopen said: \n\t%s\n\n"
				"please make sure that libmonome is installed correctly!\n",
				dlerror());
		return NULL;
	}

	protocol_new.vptr = dlsym(dl_handle, "monome_protocol_new");

	if( !protocol_new.func ) {
		fprintf(stderr, "couldn't initialize monome protocol module.  "
				"dlopen said:\n\t%s\n\n"
				"please make sure you're using a valid protocol library!\n"
				"if this is a protocol library you wrote, make sure you're"
				"providing a \033[1mmonome_protocol_new\033[0m function.\n",
				dlerror());
		goto err;
	}

	monome = (*protocol_new.func)();

	if( !monome )
		goto err;

	monome->dl_handle = dl_handle;
	return monome;

err:
	dlclose(dl_handle);
	return NULL;
}

void monome_platform_free(monome_t *monome) {
	void *dl_handle = monome->dl_handle;

	monome->free(monome);
	dlclose(dl_handle);
}
#endif

int monome_platform_open(monome_t *monome, const monome_devmap_t *m,
                         const char *dev) {
	struct termios nt, ot;
	int fd;

	if( (fd = open(dev, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0 ) {
		perror("libmonome: could not open monome device");
		return 1;
	}

	tcgetattr(fd, &ot);
	nt = ot;

	/* baud rate */
	if( m->quirks & QUIRK_57600_BAUD ) {
		cfsetispeed(&nt, B57600);
		cfsetospeed(&nt, B57600);
	} else {
		cfsetispeed(&nt, MONOME_BAUD_RATE);
		cfsetospeed(&nt, MONOME_BAUD_RATE);
	}

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

	nt.c_cc[VMIN]  = 1;
	nt.c_cc[VTIME] = 0;

	if( tcsetattr(fd, TCSANOW, &nt) < 0 )
		goto err_tcsetattr;

	tcflush(fd, TCIOFLUSH);

	monome->fd = fd;
	return 0;

err_tcsetattr:
	perror("libmonome: could not set terminal attributes");

	close(fd);
	return 1;
}

int monome_platform_close(monome_t *monome) {
	return close(monome->fd);
}

ssize_t monome_platform_write(monome_t *monome, const uint8_t *buf, size_t nbyte) {
	ssize_t ret = write(monome->fd, buf, nbyte);

	if( ret < nbyte )
		perror("libmonome: write is missing bytes");

	if( ret < 0 )
		perror("libmonome: error in write");

	return ret;
}

ssize_t monome_platform_read(monome_t *monome, uint8_t *buf, size_t nbyte) {
	ssize_t bytes, ret = 0;

	goto start;

	for( ; nbyte; nbyte -= bytes ) {
		if( monome_platform_wait_for_input(monome, READ_TIMEOUT) )
			return ret;

start:
		if ((bytes = read(monome->fd, buf, nbyte)) < 0) {
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				return 0;
			if (errno == EINTR)
				goto start;
			return bytes;
		}

		ret += bytes;
		buf += bytes;
	}

	return ret;
}

void monome_event_loop(monome_t *monome) {
	monome_callback_t *handler;
	monome_event_t e;

	fd_set fds;

	e.monome = monome;

	do {
		FD_ZERO(&fds);
		FD_SET(monome->fd, &fds);

		if( select(monome->fd + 1, &fds, NULL, NULL, NULL) < 0 ) {
			perror("libmonome: error in select()");
			break;
		}

		if( !monome->next_event(monome, &e) )
			continue;

		handler = &monome->handlers[e.event_type];
		if( !handler->cb )
			continue;

		handler->cb(&e, handler->data);
	} while( 1 );
}

void *m_malloc(size_t size) {
	return malloc(size);
}

void *m_calloc(size_t nmemb, size_t size) {
	return calloc(nmemb, size);
}

void *m_strdup(const char *s) {
	return strdup(s);
}

void m_free(void *ptr) {
	free(ptr);
}

void m_sleep(uint_t msec) {
	usleep(msec * 1000);
}
