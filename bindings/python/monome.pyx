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

	int monome_register_handler(monome_t *monome, uint event_type,
			monome_event_callback_t, void *user_data)
	int monome_unregister_handler(monome_t *monome, uint event_type)
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

cdef extern from *:
	# hack for handler_thunk
	ctypedef monome_event_t const_monome_event_t "const monome_event_t"

BUTTON_UP = 0
BUTTON_DOWN = 1
AUX_INPUT = 2

CLEAR_OFF = 0
CLEAR_ON = 1

MODE_NORMAL = 0
MODE_TEST = 1
MODE_SHUTDOWN = 2

CABLE_LEFT = 0
CABLE_BOTTOM = 1
CABLE_RIGHT = 2
CABLE_TOP = 3


cdef uint list_to_bitmap(l):
	cdef uint16_t ret = 0
	cdef uint i

	iterator = l.__iter__()

	try:
		for i from 0 <= i < 16:
			if iterator.next():
				ret |= (1 << i)
	except StopIteration:
		pass

	return ret


def _bitmap_data(data):
	if getattr(data, "__iter__", None):
		return list_to_bitmap(data)
	else:
		try:
			return int(data)
		except:
			raise TypeError("Data should be integer or iterable.")


cdef class MonomeEvent(object):
	pass


cdef class MonomeButtonEvent(MonomeEvent):
	cdef uint state, x, y

	def __cinit__(self, uint state, uint x, uint y):
		self.state = state
		self.x = x
		self.y = y
	
	property state:
		def __get__(self):
			return self.state

	property x:
		def __get__(self):
			return self.x

	property y:
		def __get__(self):
			return self.y


cdef void handler_thunk(const_monome_event_t *e, void *data):
	ev_wrapper = MonomeButtonEvent(e.event_type, e.x, e.y)
	(<Monome> data)._handlers[e.event_type](ev_wrapper)


cdef class Monome(object):
	cdef monome_t *monome
	_handlers = [None, None, None]

	orientation_map = {
		CABLE_LEFT: "left",
		CABLE_BOTTOM: "bottom",
		CABLE_RIGHT: "right",
		CABLE_TOP: "top"}

	rev_orientation_map = {
		"left": CABLE_LEFT,
		"bottom": CABLE_BOTTOM,
		"right": CABLE_RIGHT,
		"top": CABLE_TOP}

	def __init__(self, device, port=None):
		cdef char *portstr

		if device[:3] == "osc" and not port:
			raise TypeError("OSC protocol requires a server port.")

		if port:
			port = str(port)
			portstr = port

			self.monome = monome_open(device, portstr)
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

		def __set__(self, cable):
			if isinstance(cable, str):
				try:
					cable = Monome.rev_orientation_map[cable]
				except KeyError:
					raise TypeError("'%s' is not a valid cable orientation." % cable)

			monome_set_orientation(self.monome, <monome_cable_t> cable)

	property intensity:
		def __set__(self, uint intensity):
			monome_intensity(self.monome, intensity)

	property rows:
		def __get__(self):
			return monome_get_rows(self.monome)

	# "columns" seems more pythonic than "cols"
	property columns:
		def __get__(self):
			return monome_get_cols(self.monome)

	def clear(self, uint status=CLEAR_OFF):
		monome_clear(self.monome, <monome_clear_status_t> status)

	#
	# event functions
	#

	def register_handler(self, uint event_type, handler):
		if monome_register_handler(self.monome, event_type,
		                           handler_thunk, <void *> self):
			raise TypeError("Unsupported event type.")

		self._handlers[event_type] = handler

	def unregister_handler(self, uint event_type):
		if event_type not in [BUTTON_UP, BUTTON_DOWN, AUX_INPUT]:
			raise TypeError("Unsupported event type.")

		monome_unregister_handler(self.monome, event_type)
	
	def main_loop(self):
		monome_main_loop(self.monome)

	def next_event(self):
		return monome_next_event(self.monome)

	#
	# led functions
	#

	def led_on(self, uint x, uint y):
		monome_led_on(self.monome, x, y)

	def led_off(self, uint x, uint y):
		monome_led_off(self.monome, x, y)

	def led_row(self, idx, data):
		cdef uint16_t d = _bitmap_data(data)
		monome_led_row(self.monome, idx, 2, <uint8_t *> &d)

	def led_col(self, idx, data):
		cdef uint16_t d = _bitmap_data(data)
		monome_led_col(self.monome, idx, 2, <uint8_t *> &d)

	def led_frame(self, uint quadrant, rows):
		cdef uint8_t r[8]
		cdef uint16_t d
		cdef uint i

		if not getattr(rows, "__iter__", None):
			raise TypeError("'rows' must be an iterable.")

		# cython :(
		r[0] = r[1] = r[2] = r[3] = r[4] =\
			r[5] = r[6] = r[7] = 0
		rowiter = rows.__iter__()

		try:
			for i from 0 <= i < 8:
				d = _bitmap_data(rowiter.next())
				r[i] = (<uint8_t *> &d)[0]
		except StopIteration:
			pass 

		monome_led_frame(self.monome, quadrant, r)
