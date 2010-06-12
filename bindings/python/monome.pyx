# 
# Copyright (c) 2007-2010, William Light <will@visinin.com>
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
#    1. Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
# 
#    2. Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

cdef extern from "stdint.h":
	ctypedef unsigned int uint
	ctypedef char uint8_t
	ctypedef unsigned short int uint16_t

cdef extern from "monome.h":
	ctypedef struct monome_t

	ctypedef enum monome_event_type_t:
		MONOME_BUTTON_UP,
		MONOME_BUTTON_DOWN,
		MONOME_AUX_INPUT
	
	ctypedef enum monome_clear_status_t:
		MONOME_CLEAR_OFF,
		MONOME_CLEAR_ON
	
	ctypedef enum monome_mode_t:
		MONOME_MODE_NORMAL,
		MONOME_MODE_TEST,
		MONOME_MODE_SHUTDOWN
	
	ctypedef enum monome_cable_t:
		MONOME_CABLE_LEFT,
		MONOME_CABLE_BOTTOM,
		MONOME_CABLE_RIGHT,
		MONOME_CABLE_TOP

	ctypedef struct monome_event_t:
		monome_t *monome,
		monome_event_type_t event_type,
		uint x,
		uint y

	ctypedef void (*monome_event_callback_t)(monome_event_t *event, void *data)

	monome_t *monome_open(char *monome_device, ...)
	void monome_close(monome_t *monome)

	void monome_set_orientation(monome_t *monome, monome_cable_t cable)
	monome_cable_t monome_get_orientation(monome_t *monome)

	int monome_get_rows(monome_t *monome)
	int monome_get_cols(monome_t *monome)

	void monome_register_handler(monome_t *monome, uint event_type, monome_event_callback_t, void *user_data)
	void monome_unregister_handler(monome_t *monome, uint event_type)
	void monome_main_loop(monome_t *monome)
	int monome_next_event(monome_t *monome)

	int monome_clear(monome_t *monome, monome_clear_status_t status)
	int monome_intensity(monome_t *monome, uint brightness)
	int monome_mode(monome_t *monome, monome_mode_t mode)

	int monome_led_on(monome_t *monome, uint x, uint y)
	int monome_led_off(monome_t *monome, uint x, uint y)
	int monome_led_col(monome_t *monome, uint col, size_t count, uint8_t *data)
	int monome_led_row(monome_t *monome, uint row, size_t count, uint8_t *data)
	int monome_led_frame(monome_t *monome, uint quadrant, uint8_t *frame_data)

BUTTON_UP   = 0
BUTTON_DOWN = 1
AUX_INPUT   = 2

CLEAR_OFF = 0
CLAER_ON  = 1

MODE_NORMAL   = 0
MODE_TEST     = 1
MODE_SHUTDOWN = 2

CABLE_LEFT   = 0
CABLE_BOTTOM = 1
CABLE_RIGHT  = 2
CABLE_TOP    = 3

cdef uint list_to_bitmap(l):
	cdef uint i, rlen
	cdef uint16_t ret = 0
	rlen = len(l)

	for i from 0 <= i < rlen:
		if l[i]:
			ret |= 1 << i

	return ret

cdef class Monome:
	cdef monome_t *monome

	orientation_map = {
		CABLE_LEFT: "left",
		CABLE_BOTTOM: "bottom",
		CABLE_RIGHT: "right",
		CABLE_TOP: "top"
	}

	def __init__(self, device, char *port=NULL):
		if port:
			self.monome = monome_open(device, port)
		else:
			self.monome = monome_open(device)

		if not self.monome:
			raise IOError("Could not open Monome")

		self.clear(CLEAR_OFF)

	def __dealloc__(self):
		self.close()

	def close(self):
		if self.monome:
			monome_close(self.monome)

	property orientation:
		def __get__(self):
			o = <uint> monome_get_orientation(self.monome)
			return Monome.orientation_map[o]

		def __set__(self, uint cable):
			monome_set_orientation(self.monome, <monome_cable_t> cable)

	property rows:
		def __get__(self):
			return <uint> monome_get_rows(self.monome)

	property cols:
		def __get__(self):
			return <uint> monome_get_cols(self.monome)

	def clear(self, uint status):
		monome_clear(self.monome, <monome_clear_status_t> status)

	#
	# led functions
	#

	def led_on(self, uint x, uint y):
		monome_led_on(self.monome, x, y)

	def led_off(self, uint x, uint y):
		monome_led_off(self.monome, x, y)

	def led_row(self, idx, rdata):
		cdef uint16_t r

		if isinstance(rdata, int):
			r = rdata
		elif isinstance(rdata, list):
			r = list_to_bitmap(rdata)
		else:
			raise TypeError("Row data should be integer or list.")

		monome_led_row(self.monome, idx, 2, <uint8_t *> &r)
