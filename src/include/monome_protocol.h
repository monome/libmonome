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

#include <stdint.h>

#include "monome_internal.h"

int monome_protocol_populate_event(monome_event_t *event, const uint8_t *buf, const ssize_t buf_size);
