#
# Copyright (c) 2010 William Light <wrl@illest.net>
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#

from typing import Optional


cdef extern from "stdint.h":
	ctypedef unsigned int uint
	ctypedef char uint8_t
	ctypedef unsigned short int uint16_t

cdef extern from "monome.h":
	ctypedef struct monome_t

	ctypedef enum monome_event_type_t:
		MONOME_BUTTON_UP
		MONOME_BUTTON_DOWN
		MONOME_ENCODER_DELTA
		MONOME_ENCODER_KEY_UP
		MONOME_ENCODER_KEY_DOWN
		MONOME_TILT

	ctypedef enum monome_rotate_t:
		MONOME_CABLE_LEFT
		MONOME_CABLE_BOTTOM
		MONOME_CABLE_RIGHT
		MONOME_CABLE_TOP

	cdef struct _ev_grid:
		uint x
		uint y

	cdef struct _ev_encoder:
		uint number
		int delta

	cdef struct _ev_tilt:
		uint sensor
		int x
		int y
		int z

	ctypedef struct monome_event_t:
		monome_t *monome
		monome_event_type_t event_type
		_ev_grid grid
		_ev_encoder encoder
		_ev_tilt tilt

	ctypedef void (*monome_event_callback_t)(const monome_event_t *event, void *data)

	monome_t *monome_open(char *monome_device, ...)
	void monome_close(monome_t *monome)

	void monome_set_rotation(monome_t *monome, monome_rotate_t cable)
	monome_rotate_t monome_get_rotation(monome_t *monome)

	const char * monome_get_serial(monome_t *monome)
	const char * monome_get_devpath(monome_t *monome)
	int monome_get_rows(monome_t *monome)
	int monome_get_cols(monome_t *monome)

	int monome_register_handler(monome_t *monome, monome_event_type_t event_type, monome_event_callback_t, void *user_data)
	int monome_unregister_handler(monome_t *monome, monome_event_type_t event_type)
	void monome_event_loop(monome_t *monome)
	int monome_event_next(monome_t *monome, monome_event_t *event_buf)
	int monome_event_handle_next(monome_t *monome)
	int monome_get_fd(monome_t *monome)

	int monome_led_intensity(monome_t *monome, uint brightness)

	int monome_led_on(monome_t *monome, uint x, uint y)
	int monome_led_off(monome_t *monome, uint x, uint y)
	int monome_led_all(monome_t *monome, uint status)
	int monome_led_row(monome_t *monome, uint x_off, uint y, size_t count, uint8_t *data)
	int monome_led_col(monome_t *monome, uint x, uint y_off, size_t count, uint8_t *data)
	int monome_led_map(monome_t *monome, uint x_off, uint y_off, uint8_t *data)

	int monome_led_ring_set(monome_t *monome, unsigned int ring, unsigned int led, unsigned int level)
	int monome_led_ring_all(monome_t *monome, unsigned int ring, unsigned int level)
	int monome_led_ring_map(monome_t *monome, unsigned int ring, const uint8_t *levels)
	int monome_led_ring_range(monome_t *monome, unsigned int ring, unsigned int start, unsigned int end, unsigned int level)

__all__ = [
	# constants
	# XXX: should these be members of the class?

	"BUTTON_UP",
	"BUTTON_DOWN",
	"ENCODER_DELTA",
	"ENCODER_KEY_UP",
	"ENCODER_KEY_DOWN",
	"TILT",
	"CABLE_LEFT",
	"CABLE_BOTTOM",
	"CABLE_RIGHT",
	"CABLE_TOP",
	"MODE_NORMAL",
	"MODE_TEST",
	"MODE_SHUTDOWN"
	"ROTATE_0",
	"ROTATE_90",
	"ROTATE_180",
	"ROTATE_270",

	# classes

	"MonomeEvent",
	"MonomeGridEvent",
	"MonomeEncoderKeyEvent",
	"MonomeEncoderEvent",
	"Monome",
]

cpdef enum:
	BUTTON_UP
	BUTTON_DOWN
	ENCODER_DELTA
	ENCODER_KEY_UP
	ENCODER_KEY_DOWN
	TILT

cpdef enum:
	CABLE_LEFT
	CABLE_BOTTOM
	CABLE_RIGHT
	CABLE_TOP

cpdef enum:
	MODE_NORMAL
	MODE_TEST
	MODE_SHUTDOWN

cpdef enum:
	ROTATE_0
	ROTATE_90
	ROTATE_180
	ROTATE_270


cdef uint list_to_bitmap(l) except *:
	cdef uint16_t ret = 0
	cdef uint i

	iterator = iter(l)

	try:
		for i in range(16):
			if iterator.next():
				ret |= (1 << i)
	except StopIteration:
		pass

	return ret

def _bitmap_data(list[int] data):
	try:
		return list_to_bitmap(data)
	except TypeError:
		try:
			return int(data)
		except:
			raise TypeError("'%s' object is neither iterable nor integer." % type(data).__name__)


cdef class MonomeEvent:
	cdef object monome

cdef class MonomeGridEvent(MonomeEvent):
	cdef uint x, y
	cdef bint pressed

	def __cinit__(self, bint pressed, uint x, uint y, object monome):
		self.monome = monome
		self.pressed = pressed
		self.x = x
		self.y = y

	def __repr__(self):
		return "%s(%s, %d, %d)" % \
			(self.__class__.__name__, self.pressed, self.x, self.y)

	@property 
	def monome(self) -> object:
		return self.monome

	@property 
	def pressed(self) -> bool:
		return self.pressed

	@property 
	def x(self) -> uint:
		return self.x

	@property 
	def y(self) -> uint:
		return self.y

cdef class MonomeEncoderKeyEvent(MonomeEvent):
	cdef bint pressed
	cdef uint number

	def __cinit__(self, bint pressed, uint number, object monome):
		self.monome = monome
		self.pressed = pressed
		self.number = number

	def __repr__(self):
		return "%s(%d, %d)" % \
				(self.__class__.__name__, self.pressed, self.number)

	@property 
	def monome(self) -> object:
		return self.monome

	@property 
	def pressed(self) -> bool:
		return self.pressed

	@property 
	def number(self) -> int:
		return self.number

cdef class MonomeEncoderEvent(MonomeEvent):
	cdef uint number
	cdef int delta

	def __cinit__(self, uint number, int delta, object monome):
		self.monome = monome
		self.number = number
		self.delta = delta

	def __repr__(self):
		return "%s(%s, %d)" % \
				(self.__class__.__name__, self.number, self.delta)

	@property 
	def monome(self) -> object:
		return self.monome

	@property 
	def number(self) -> uint :
		return self.number

	@property 
	def delta(self) -> int:
		return self.delta


cdef MonomeEvent event_from_event_t(const monome_event_t *e, object monome=None):
	if e.event_type == MONOME_BUTTON_DOWN:
		return MonomeGridEvent(1, e.grid.x, e.grid.y, monome)
	elif e.event_type == MONOME_BUTTON_UP:
		return MonomeGridEvent(0, e.grid.x, e.grid.y, monome)
	elif e.event_type == MONOME_ENCODER_DELTA:
		return MonomeEncoderEvent(e.encoder.number, e.encoder.delta, monome)
	elif e.event_type == MONOME_ENCODER_KEY_DOWN:
		return MonomeEncoderKeyEvent(1, e.encoder.number, monome)
	elif e.event_type == MONOME_ENCODER_KEY_UP:
		return MonomeEncoderKeyEvent(0, e.encoder.number, monome)
	else:
		raise RuntimeError('Unknown or unimplemented event_type {}'.format(e.event_type))

	# XXX: handle other event types

cdef void handler_thunk(const monome_event_t *event, void *data) noexcept:
	ev_wrapper = event_from_event_t(event, (<Monome> data))
	(<Monome> data).handlers[event.event_type](ev_wrapper)

cdef enum:
	ARC_RING_SIZE = 64

def check_level(level):
	if level < 0 or level > 15:
		raise ValueError('Ring LED level {} is out of bounds, must be [0, 15]'.format(level))

	return level


cdef class Monome:
	cdef monome_t *monome

	cdef str serial
	cdef str devpath
	cdef int fd
	cdef list handlers

	rotation_map = {
		ROTATE_0: 0,
		ROTATE_90: 90,
		ROTATE_180: 180,
		ROTATE_270: 270}

	rev_rotation_map = {
		0: ROTATE_0,
		90: ROTATE_90,
		180: ROTATE_180,
		270: ROTATE_270}

	def __init__(self, str device, int port=0, bint clear=True):
		if device[:3] == "osc" and not port:
			raise TypeError("OSC protocol requires a server port.")

		if port:
			self.monome = monome_open(device.encode(), str(port).encode())
		else:
			self.monome = monome_open(device.encode())

		if self.monome is NULL:
			raise IOError("Could not open Monome")

		cdef const char * ser = monome_get_serial(self.monome)

		self.serial = ser.decode() if ser else None
		self.devpath = monome_get_devpath(self.monome).decode()
		self.fd = monome_get_fd(self.monome)
		self.handlers = [None, None, None]

		if clear:
			self.clear()

	def __dealloc__(self):
		if self.monome is not NULL:
			monome_close(self.monome)

	@property
	def rotation(self) -> uint:
		o = <uint> monome_get_rotation(self.monome)
		return Monome.rotation_map[o]

	@rotation.setter
	def rotation(self, uint rotation):
		if rotation > 3:
			try:
				rotation = Monome.rev_rotation_map[rotation]
			except KeyError:
				raise TypeError("'%s' is not a valid rotation." % rotation)

		monome_set_rotation(self.monome, <monome_rotate_t> rotation)

	@property
	def serial(self) -> str:
		return self.serial

	@property
	def devpath(self) -> str:
		return self.devpath

	@property
	def rows(self) -> int:
		return monome_get_rows(self.monome)

	# "columns" seems more pythonic than "cols"
	@property
	def columns(self) -> int:
		return monome_get_cols(self.monome)

	#
	# event functions
	#

	def register_handler(self, monome_event_type_t event_type, handler):
		if not callable(handler):
			raise TypeError("'%s' object is not callable." % type(handler).__name__)

		# monome_register_handler returns 0 on success, EINVAL when
		# passed an invalid event type.
		if monome_register_handler(self.monome, event_type,
								   handler_thunk, <void *> self):
			raise TypeError("Unsupported event type.")

		self.handlers[event_type] = handler

	def unregister_handler(self, monome_event_type_t event_type):
		if monome_unregister_handler(self.monome, event_type):
			raise TypeError("Unsupported event type.")

	def event_loop(self):
		monome_event_loop(self.monome)

	def handle_next_event(self) -> int:
		return monome_event_handle_next(self.monome)

	def next_event(self) -> Optional[MonomeEvent]:
		cdef monome_event_t e

		if monome_event_next(self.monome, &e):
			return event_from_event_t(&e, self)

		return None

	def fileno(self) -> int:
		return self.fd

	#
	# led functions
	#

	def set_led_intensity(self, uint intensity):
		monome_led_intensity(self.monome, intensity)
	led_intensity = property(fset=set_led_intensity)

	def led_on(self, uint x, uint y):
		monome_led_on(self.monome, x, y)

	def led_off(self, uint x, uint y):
		monome_led_off(self.monome, x, y)

	def led_all(self, uint status=0):
		monome_led_all(self.monome, status)

	def led_row(self, uint x_off, uint y, list[int] data):
		cdef uint16_t d = _bitmap_data(data)
		monome_led_row(self.monome, x_off, y, 2, <uint8_t *> &d)

	def led_col(self, uint x, uint y_off, list[int] data):
		cdef uint16_t d = _bitmap_data(data)
		monome_led_col(self.monome, x, y_off, 2, <uint8_t *> &d)

	def led_map(self, uint x_off, uint y_off, list[int] data):
		cdef uint8_t r[8]
		cdef uint16_t d
		cdef uint i

		# will raise a TypeError if rows is not iterable
		data_iter = iter(data)

		# cython :(
		r[0] = r[1] = r[2] = r[3] = r[4] = r[5] = r[6] = r[7] = 0

		try:
			for i in range(8):
				d = _bitmap_data(data_iter.next())
				r[i] = (<uint8_t *> &d)[0]
		except StopIteration:
			raise ValueError('data map contained insuffient number of cols')

		monome_led_map(self.monome, x_off, y_off, r)

	def led_ring_set(self, uint ring, uint led, uint level):
		level = check_level(level)
		monome_led_ring_set(self.monome, ring, led, level)

	def led_ring_all(self, uint ring, uint level):
		level = check_level(level)
		monome_led_ring_all(self.monome, ring, level)

	def led_ring_map(self, uint ring, list[int] levels):
		cdef uint8_t levels_arr[ARC_RING_SIZE]

		levels_iter = iter(levels)

		for idx in range(ARC_RING_SIZE):
			level = next(levels_iter)
			level = check_level(level)
			levels_arr[idx] = level

		monome_led_ring_map(self.monome, ring, levels_arr)

	def led_ring_range(self, uint ring, uint start, uint end, uint level):
		level = check_level(level)
		monome_led_ring_range(self.monome, ring, start, end, level)

	def clear(self):
		self.led_all(0)

