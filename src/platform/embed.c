/**
 * Copyright (c) 2013 Nedko Arnaudov <nedko@arnaudov.name>
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

#include <monome.h>
#include "protocol.h"
#include "platform.h"

#include <string.h>

struct monome_proto
{
	const char *proto;
	monome_proto_new_func_t new;
};

static struct monome_proto g_protos[] = {
	{ "40h",    monome_protocol_40h_new },
	{ "series", monome_protocol_series_new },
	{ "mext",   monome_protocol_mext_new },
#if defined(BUILD_OSC_PROTO)
	{ "osc",    monome_protocol_osc_new },
#endif
	{ NULL }
};

monome_t *monome_platform_load_protocol(const char *proto) {
	struct monome_proto *proto_ptr;

	for( proto_ptr = g_protos; proto_ptr->proto != NULL; proto_ptr++ )
		if (strcmp(proto_ptr->proto, proto) == 0)
			return proto_ptr->new();

	return NULL;
}

void monome_platform_free(monome_t *monome) {
	monome->free(monome);
}
