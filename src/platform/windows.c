/**
 * Copyright (c) 2011 William Light <wrl@illest.net>
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

#ifdef __STRICT_ANSI__
#undef __STRICT_ANSI__
#endif

#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>

/* THIS WAY LIES MADNESS */
#include <windows.h>
#include <Winreg.h>
#include <io.h>

#include <monome.h>
#include "internal.h"
#include "platform.h"

#define FTDI_REG_PATH "SYSTEM\\CurrentControlSet\\Enum\\FTDIBUS"

static char *m_asprintf(const char *fmt, ...) {
	va_list args;
	char *buf;
	int len;

	va_start(args, fmt);

	len = _vscprintf(fmt, args) + 1;
	if( !(buf = m_calloc(sizeof(char), len)) )
		return NULL;

	vsprintf(buf, fmt, args);
	va_end(args);

	return buf;
}

monome_t *monome_platform_load_protocol(const char *proto) {
	typedef monome_t *(*proto_init_t)();
	proto_init_t protocol_new;

	monome_t *monome;
	HMODULE proto_mod;
	char *modname;

	if( !(modname = m_asprintf("monome\\protocol_%s.dll", proto)) )
		goto err_loadlibrary;

	proto_mod = LoadLibrary(modname);
	m_free(modname);

	if( !proto_mod )
		goto err_loadlibrary;

	protocol_new = (proto_init_t) GetProcAddress(proto_mod, "monome_protocol_new");

	if( !protocol_new )
		goto err_protocol_new;

	if( !(monome = protocol_new()) )
		goto err_protocol_new;

	monome->dl_handle = proto_mod;
	return monome;

err_protocol_new:
	FreeLibrary(proto_mod);
err_loadlibrary:
	return NULL;
}

void monome_platform_free(monome_t *monome) {
	void *dl_handle = monome->dl_handle;

	monome->free(monome);
	FreeLibrary(dl_handle);
}

int monome_platform_open(monome_t *monome, const monome_devmap_t *m,
                         const char *dev) {
	DCB serparm = {0};
	char *devesc;
	HANDLE hser;
	COMMTIMEOUTS timeouts = {
		.ReadIntervalTimeout         = MAXDWORD,
		.ReadTotalTimeoutConstant    = 0,
		.ReadTotalTimeoutMultiplier  = 0,
		.WriteTotalTimeoutConstant   = 0,
		.WriteTotalTimeoutMultiplier = 0
	};

	if( !(devesc = m_asprintf("\\\\.\\%s", dev)) ) {
		fprintf(stderr, "libmonome: could not open %s: out of memory\n", dev);
		return 1;
	}

	hser = CreateFile(devesc, GENERIC_READ | GENERIC_WRITE, 0, NULL,
	                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL |
	                  FILE_FLAG_OVERLAPPED, 0);

	free(devesc);

	if( hser == INVALID_HANDLE_VALUE )
		goto err_open;

	serparm.DCBlength = sizeof(serparm);

	if( !GetCommState(hser, &serparm) )
		goto err_commstate;

	if( m->quirks & QUIRK_57600_BAUD )
		serparm.BaudRate = CBR_57600;
	else
		serparm.BaudRate = CBR_115200;

	serparm.ByteSize = 8;
	serparm.StopBits = ONESTOPBIT;
	serparm.Parity   = NOPARITY;
	serparm.fBinary  = 1;

	if( !SetCommState(hser, &serparm) )
		goto err_commstate;

	if( !SetCommTimeouts(hser, &timeouts) )
		goto err_commstate;

	PurgeComm(hser, PURGE_RXCLEAR | PURGE_TXCLEAR);

	monome->fd = _open_osfhandle((intptr_t) hser, _O_RDWR | _O_BINARY);
	return 0;

err_commstate:
	CloseHandle(hser);
err_open:
	if( GetLastError() != ERROR_FILE_NOT_FOUND )
		printf("libmonome: could not open %s: error %ld", dev,
		       GetLastError());
	return 1;
}

int monome_platform_close(monome_t *monome) {
	return !!_close(monome->fd);
}

ssize_t monome_platform_write(monome_t *monome, const uint8_t *buf, size_t nbyte) {
	HANDLE hres = (HANDLE) _get_osfhandle(monome->fd);
	OVERLAPPED ov = {0, 0, {{0, 0}}};
	DWORD written = 0;

	if( !(ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) ) {
		fprintf(stderr,
				"monome_plaform_write(): could not allocate event (%ld)\n",
				GetLastError());
		return -1;
	}

	if( !WriteFile(hres, buf, nbyte, &written, &ov) ) {
		if( GetLastError() != ERROR_IO_PENDING ) {
			fprintf(stderr, "monome_platform_write(): write failed (%ld)\n",
					GetLastError());
			return -1;
		}

		GetOverlappedResult(hres, &ov, &written, TRUE);
	}

	CloseHandle(ov.hEvent);
	return written;
}

ssize_t monome_platform_read(monome_t *monome, uint8_t *buf, size_t nbyte) {
	HANDLE hres = (HANDLE) _get_osfhandle(monome->fd);
	OVERLAPPED ov = {0, 0, {{0, 0}}};
	DWORD read = 0;

	if( !(ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)) ) {
		fprintf(stderr,
				"monome_plaform_read(): could not allocate event (%ld)\n",
				GetLastError());
		return -1;
	}

	if( !ReadFile(hres, buf, nbyte, &read, &ov) ) {
		if( GetLastError() != ERROR_IO_PENDING ) {
			fprintf(stderr, "monome_platform_read(): read failed (%ld)\n",
					GetLastError());
			return -1;
		}

		GetOverlappedResult(hres, &ov, &read, TRUE);
	}

	CloseHandle(ov.hEvent);
	return read;
}

char *monome_platform_get_dev_serial(const char *path) {
	HKEY key, subkey;
	char subkey_name[MAX_PATH], *subkey_path, *serial;
	unsigned char port_name[64];
	DWORD klen, plen, ptype;
	int i = 0;

	serial = NULL;

	switch( RegOpenKeyEx(
			HKEY_LOCAL_MACHINE, FTDI_REG_PATH,
			0, KEY_READ, &key) ) {
	case ERROR_SUCCESS:
		/* ERROR: request was (unexpectedly) successful */
		break;

	case ERROR_FILE_NOT_FOUND:
		/* print message about needing the FTDI driver maybe? */
		/* fall through also */
	default:
		return NULL;
	}

	do {
		klen = sizeof(subkey_name) / sizeof(char);
		switch( RegEnumKeyEx(key, i++, subkey_name, &klen,
							 NULL, NULL, NULL, NULL) ) {
		case ERROR_MORE_DATA:
		case ERROR_SUCCESS:
			break;

		default:
			goto done;
		}

		subkey_path = m_asprintf("%s\\%s\\0000\\Device Parameters",
								 FTDI_REG_PATH, subkey_name);

		switch( RegOpenKeyEx(
				HKEY_LOCAL_MACHINE, subkey_path,
				0, KEY_READ, &subkey) ) {
		case ERROR_SUCCESS:
			break;

		default:
			continue;
		}

		free(subkey_path);

		plen = sizeof(port_name) / sizeof(char);
		ptype = REG_SZ;
		switch( RegQueryValueEx(subkey, "PortName", 0, &ptype,
								port_name, &plen) ) {
		case ERROR_SUCCESS:
			port_name[plen] = '\0';
			break;

		default:
			goto nomatch;
		}

		if( !strcmp((char *) port_name, path) ) {
			/* there's a fucking "A" right after the serial number */
			subkey_name[klen - 1] = '\0';
			serial = strrchr(subkey_name, '+') + 1;

			RegCloseKey(subkey);
			break;
		}

nomatch:
		RegCloseKey(subkey);
	} while( 1 );

done:
	RegCloseKey(key);
	return ( serial ) ? strdup(serial) : NULL;
}

int monome_platform_wait_for_input(monome_t *monome, uint_t msec) {
	Sleep(msec); /* fuck it */
	return 1;
}

void monome_event_loop(monome_t *monome) {
	printf("monome_event_loop() is unimplemented\n");
	return;
}

void *m_malloc(size_t size) {
	return malloc(size);
}

void *m_calloc(size_t nmemb, size_t size) {
	return calloc(nmemb, size);
}

void *m_strdup(const char *s) {
	return _strdup(s);
}

void m_free(void *ptr) {
	free(ptr);
}

void m_sleep(uint_t msec) {
	Sleep(msec);
}
