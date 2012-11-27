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

from cpython cimport bool

cdef extern from "stdint.h":
	ctypedef unsigned int uint
	ctypedef char uint8_t
	ctypedef unsigned short int uint16_t

cdef extern from "monome.h":
	ctypedef struct monome_t

	ctypedef enum monome_event_type_t:
		MONOME_BUTTON_UP,
		MONOME_BUTTON_DOWN,
		MONOME_ENCODER_DELTA,
		MONOME_ENCODER_KEY_UP,
		MONOME_ENCODER_KEY_DOWN,
		MONOME_TILT

	ctypedef enum monome_rotate_t:
		MONOME_CABLE_LEFT,
		MONOME_CABLE_BOTTOM,
		MONOME_CABLE_RIGHT,
		MONOME_CABLE_TOP

	cdef struct _ev_grid:
		uint x,
		uint y

	cdef struct _ev_encoder:
		uint number,
		int delta

	cdef struct _ev_tilt:
		uint sensor,
		int x,
		int y,
		int z

	ctypedef struct monome_event_t:
		monome_t *monome,
		monome_event_type_t event_type,

		_ev_grid grid,
		_ev_encoder encoder,
		_ev_tilt tilt

	# const hackery
	ctypedef monome_event_t const_monome_event_t "const monome_event_t"
	ctypedef void (*monome_event_callback_t)(const_monome_event_t *event, void *data)

	monome_t *monome_open(char *monome_device, ...)
	void monome_close(monome_t *monome)

	void monome_set_rotation(monome_t *monome, monome_rotate_t cable)
	monome_rotate_t monome_get_rotation(monome_t *monome)

	# more const hackery
	ctypedef char * const_char_p "const char *"
	const_char_p monome_get_serial(monome_t *monome)
	const_char_p monome_get_devpath(monome_t *monome)
	int monome_get_rows(monome_t *monome)
	int monome_get_cols(monome_t *monome)

	int monome_register_handler(monome_t *monome,
			monome_event_type_t event_type, monome_event_callback_t,
			void *user_data)
	int monome_unregister_handler(monome_t *monome,
			monome_event_type_t event_type)
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

all = [
	# constants
	# XXX: should these be members of the class?

	"MODE_NORMAL",
	"MODE_TEST",
	"MODE_SHUTDOWN",
	"CABLE_LEFT",
	"CABLE_BOTTOM",
	"CABLE_RIGHT",
	"CABLE_TOP",

	# classes

	"MonomeEvent",
	"MonomeGridEvent",

	"Monome"]

BUTTON_UP, BUTTON_DOWN, ENCODER_DELTA, ENCODER_KEY_UP, ENCODER_KEY_DOWN, TILT = range(6)
CABLE_LEFT, CABLE_BOTTOM, CABLE_RIGHT, CABLE_TOP = range(4)

MODE_NORMAL = 0
MODE_TEST = 1
MODE_SHUTDOWN = 2

ROTATE_0 = 0
ROTATE_90 = 1
ROTATE_180 = 2
ROTATE_270 = 3


cdef uint list_to_bitmap(l) except *:
	cdef uint16_t ret = 0
	cdef uint i

	iterator = iter(l)

	try:
		for i from 0 <= i < 16:
			if iterator.next():
				ret |= (1 << i)
	except StopIteration:
		pass

	return ret


def _bitmap_data(data):
	try:
		return list_to_bitmap(data)
	except TypeError:
		try:
			return int(data)
		except:
			raise TypeError("'%s' object is neither iterable nor integer." % type(data).__name__)


cdef class MonomeEvent(object):
	cdef object monome

cdef class MonomeGridEvent(MonomeEvent):
	cdef uint x, y
	cdef bool pressed

	def __cinit__(self, pressed, uint x, uint y, object monome):
		self.monome = monome
		self.pressed = bool(pressed)
		self.x = x
		self.y = y

	def __repr__(self):
		return "%s(%s, %d, %d)" % \
				(self.__class__.__name__, self.pressed, self.x, self.y)

	property monome:
		def __get__(self):
			return self.monome

	property pressed:
		def __get__(self):
			return self.pressed

	property x:
		def __get__(self):
			return self.x

	property y:
		def __get__(self):
			return self.y


cdef MonomeEvent event_from_event_t(const_monome_event_t *e, object monome=None):
	if   e.event_type == MONOME_BUTTON_DOWN:
		return MonomeGridEvent(1, e.grid.x, e.grid.y, monome)
	elif e.event_type == MONOME_BUTTON_UP:
		return MonomeGridEvent(0, e.grid.x, e.grid.y, monome)

	# XXX: handle other event types

cdef void handler_thunk(const_monome_event_t *event, void *data):
	ev_wrapper = event_from_event_t(event, (<Monome> data))
	(<Monome> data).handlers[event.event_type](ev_wrapper)


cdef class Monome(object):
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

	def __init__(self, device, port=None, clear=True):
		cdef char *portstr
		cdef const_char_p ser

		if device[:3] == "osc" and not port:
			raise TypeError("OSC protocol requires a server port.")

		dbytes = device.encode()

		if port:
			port = port.encode()
			portstr = port

			self.monome = monome_open(dbytes, portstr)
		else:
			self.monome = monome_open(dbytes)

		if not self.monome:
			raise IOError("Could not open Monome")

		ser = monome_get_serial(self.monome)

		self.serial = ser.decode() if ser else None
		self.devpath = (monome_get_devpath(self.monome)).decode()
		self.fd = monome_get_fd(self.monome)
		self.handlers = [None, None, None]

		if clear:
			self.led_all(0)

	def __dealloc__(self):
		if self.monome:
			monome_close(self.monome)

	property rotation:
		def __get__(self):
			o = <uint> monome_get_rotation(self.monome)
			return Monome.rotation_map[o]

		def __set__(self, uint rotation):
			if rotation > 3:
				try:
					rotation = Monome.rev_rotation_map[rotation]
				except KeyError:
					raise TypeError("'%s' is not a valid rotation." % rotation)

			monome_set_rotation(self.monome, <monome_rotate_t> rotation)

	property serial:
		def __get__(self):
			return self.serial

	property devpath:
		def __get__(self):
			return self.devpath

	@property
	def rows(self):
		return monome_get_rows(self.monome)

	# "columns" seems more pythonic than "cols"
	@property
	def columns(self):
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

	def handle_next_event(self):
		if monome_event_handle_next(self.monome):
			return True
		else:
			return False

	def next_event(self):
		cdef monome_event_t e

		if monome_event_next(self.monome, &e):
			return event_from_event_t(&e, self)

		return None

	def fileno(self):
		return self.fd

	#
	# led functions
	#

	property led_intensity:
		def __set__(self, uint intensity):
			monome_led_intensity(self.monome, intensity)

	def led_on(self, uint x, uint y):
		monome_led_on(self.monome, x, y)

	def led_off(self, uint x, uint y):
		monome_led_off(self.monome, x, y)

	def led_all(self, uint status=0):
		monome_led_all(self.monome, status)

	def led_row(self, x_off, y, data):
		cdef uint16_t d = _bitmap_data(data)
		monome_led_row(self.monome, x_off, y, 2, <uint8_t *> &d)

	def led_col(self, x, y_off, idx, data):
		cdef uint16_t d = _bitmap_data(data)
		monome_led_col(self.monome, x, y_off, 2, <uint8_t *> &d)

	def led_map(self, uint x_off, uint y_off, data):
		cdef uint8_t r[8]
		cdef uint16_t d
		cdef uint i

		# will raise a TypeError if rows is not iterable
		data_iter = iter(data)

		# cython :(
		r[0] = r[1] = r[2] = r[3] = r[4] =\
			r[5] = r[6] = r[7] = 0

		try:
			for i from 0 <= i < 8:
				d = _bitmap_data(data_iter.next())
				r[i] = (<uint8_t *> &d)[0]
		except StopIteration:
			pass

		monome_led_map(self.monome, x_off, y_off, r)
