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

#include "internal.h"

char *monome_platform_get_dev_serial(const char *device);

monome_t *monome_platform_load_protocol(const char *proto);
void monome_platform_free(monome_t *monome);

int monome_platform_open(monome_t *monome, const monome_devmap_t *m,
                         const char *dev);
int monome_platform_close(monome_t *monome);

ssize_t monome_platform_write(monome_t *monome, const uint8_t *buf, size_t nbyte);
ssize_t monome_platform_read(monome_t *monome, uint8_t *buf, size_t nbyte);

int monome_platform_wait_for_input(monome_t *monome, uint_t msec);

void *m_malloc(size_t size);
void *m_calloc(size_t nmemb, size_t size);
void *m_strdup(const char *s);
void m_free(void *ptr);
void m_sleep(uint_t msec);
