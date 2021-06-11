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
#include <setupapi.h>
#include <cfgmgr32.h>
#include <initguid.h>

#include <monome.h>
#include "internal.h"
#include "platform.h"

DEFINE_GUID(GUID_DEVINTERFACE_COMPORT, 0x86e0d1e0L, 0x8089, 0x11d0, 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73);

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

static char *m_get_serial_from_instance_id(const char *instance_id) {
	char serial[MAX_PATH];

	if (strncmp(instance_id, "FTDIBUS\\", 8) == 0) {
		if (sscanf(instance_id, "FTDIBUS\\VID_%*x+PID_%*x+%[a-zA-Z0-9]m", serial) == 1) {
			/* clear the "A" right after the serial number */
			serial[strlen(serial) - 1] = '\0';
			return strdup(serial);
		}
	} else if (strncmp(instance_id, "USB\\", 4) == 0) {
		if (sscanf(instance_id,"USB\\VID_%*x&PID_%*x\\%[a-zA-Z0-9]m", serial) == 1) {
			return strdup(serial);
		}
	}

	fprintf(stderr, "libmonome: failed to parse device instance id: %s\n", instance_id);
	return NULL;
}

static char *m_get_device_port_name(HDEVINFO hdevinfo, SP_DEVINFO_DATA *devinfo) {
	DWORD port_name_size;
	unsigned char port_name[MAX_PATH];
	HKEY hkey;
	LSTATUS status;

	hkey = SetupDiOpenDevRegKey(hdevinfo, devinfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
	status = RegQueryValueEx(hkey, "PortName", NULL, NULL, port_name, &port_name_size);

	if (status == ERROR_SUCCESS) {
		port_name[port_name_size] = '\0';

		RegCloseKey(hkey);
		return strdup((char *) port_name);
	}

	RegCloseKey(hkey);
	return NULL;
}

monome_t *monome_platform_load_protocol(const char *proto) {
	monome_proto_new_func_t protocol_new;

	monome_t *monome;
	HMODULE proto_mod;
	char *modname;

	if( !(modname = m_asprintf("monome\\protocol_%s.dll", proto)) )
		goto err_loadlibrary;

	proto_mod = LoadLibrary(modname);
	m_free(modname);

	if( !proto_mod )
		goto err_loadlibrary;

	protocol_new = (monome_proto_new_func_t) GetProcAddress(proto_mod, "monome_protocol_new");

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
	HDEVINFO hdevinfo;
	SP_DEVINFO_DATA devinfo;
	char *serial;
	int di;

	serial = NULL;

	hdevinfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_COMPORT, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

	if (hdevinfo == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "libmonome: SetupDiGetClassDevs() failed.\n");
		return NULL;
	}

	devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
	di = 0;

	while (SetupDiEnumDeviceInfo(hdevinfo, di, &devinfo)) {
		char *port_name;
		port_name = m_get_device_port_name(hdevinfo, &devinfo);

		if (strcmp(port_name, path) == 0) {
			char instance_id[MAX_DEVICE_ID_LEN];
			DWORD instance_id_size = sizeof(instance_id);

			if (!SetupDiGetDeviceInstanceId(hdevinfo, &devinfo, instance_id, instance_id_size, NULL)) {
				fprintf(stderr, "libmonome: SetupDiGetDeviceInstanceId() failed.\n");
				continue;
			};

			serial = m_get_serial_from_instance_id(instance_id);
		}

		di++;
	}

	SetupDiDestroyDeviceInfoList(hdevinfo);

	return serial;
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
