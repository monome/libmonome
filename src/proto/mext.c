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

#include <stdlib.h>

#include <monome.h>
#include "internal.h"

#include "mext.h"


#define SELF_FROM(monome) mext_t *self = (mext_t *) monome


static void mext_free(monome_t *monome) {
	SELF_FROM(monome);

	free(self);
}

monome_t *monome_protocol_new() {
	mext_t *self = calloc(1, sizeof(mext_t));
	monome_t *monome = self;

	if( !monome )
		return NULL;

	monome->open       = NULL;
	monome->close      = NULL;
	monome->free       = mext_free;

	monome->next_event = NULL;

	monome->clear      = NULL;
	monome->intensity  = NULL;
	monome->mode       = NULL;
	
	monome->led        = NULL;
	monome->led_col    = NULL;
	monome->led_row    = NULL;
	monome->led_frame  = NULL;


	return NULL;
}
