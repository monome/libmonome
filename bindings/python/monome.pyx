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

	# const hackery
	ctypedef monome_event_t const_monome_event_t "const monome_event_t"
	ctypedef void (*monome_event_callback_t)(const_monome_event_t *event, void *data)

	monome_t *monome_open(char *monome_device, ...)
	void monome_close(monome_t *monome)

	void monome_set_orientation(monome_t *monome, monome_cable_t cable)
	monome_cable_t monome_get_orientation(monome_t *monome)

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

	int monome_clear(monome_t *monome, monome_clear_status_t status)
	int monome_intensity(monome_t *monome, uint brightness)
	int monome_mode(monome_t *monome, monome_mode_t mode)

	int monome_led_on(monome_t *monome, uint x, uint y)
	int monome_led_off(monome_t *monome, uint x, uint y)
	int monome_led_col(monome_t *monome, uint col, size_t count, uint8_t *data)
	int monome_led_row(monome_t *monome, uint row, size_t count, uint8_t *data)
	int monome_led_frame(monome_t *monome, uint quadrant, uint8_t *frame_data)

all = [
	# constants
	# XXX: should these be members of the class?

	"BUTTON_UP",
	"BUTTON_DOWN",
	"AUX_INPUT",
	"CLEAR_OFF",
	"CLEAR_ON",
	"MODE_NORMAL",
	"MODE_TEST",
	"MODE_SHUTDOWN",
	"CABLE_LEFT",
	"CABLE_BOTTOM",
	"CABLE_RIGHT",
	"CABLE_TOP",

	# classes

	"MonomeEvent",
	"MonomeButtonEvent",

	"Monome"]


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
	pass


cdef class MonomeButtonEvent(MonomeEvent):
	cdef uint x, y
	cdef bool pressed
	cdef object monome

	def __cinit__(self, pressed, uint x, uint y, object monome=None):
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
	return MonomeButtonEvent(<uint> e.event_type, e.x, e.y, monome)

cdef void handler_thunk(const_monome_event_t *event, void *data):
	ev_wrapper = event_from_event_t(event, (<Monome> data))
	(<Monome> data).handlers[event.event_type](ev_wrapper)


cdef class Monome(object):
	cdef monome_t *monome

	cdef str serial
	cdef str devpath
	cdef int fd
	cdef list handlers

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

	rev_mode_map = {
		"normal": MODE_NORMAL,
		"shutdown": MODE_SHUTDOWN,
		"test": MODE_TEST}

	def __init__(self, device, port=None, clear=True):
		cdef char *portstr
		cdef const_char_p ser

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

		ser = monome_get_serial(self.monome)

		self.serial = ser if ser else None
		self.devpath = monome_get_devpath(self.monome)
		self.fd = monome_get_fd(self.monome)
		self.handlers = [None, None, None]

		if clear:
			self.clear(CLEAR_OFF)

	def __dealloc__(self):
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

		if not monome_event_next(self.monome, &e):
			return None
		else:
			return event_from_event_t(&e, self)

	def fileno(self):
		return self.fd

	#
	# led functions
	#

	def clear(self, uint status=CLEAR_OFF):
		monome_clear(self.monome, <monome_clear_status_t> status)

	property mode:
		def __set__(self, mode):
			if isinstance(mode, str):
				try:
					monome_mode(self.monome,
						<monome_mode_t> Monome.rev_mode_map[mode])
				except KeyError:
					raise TypeError("'%s' is not a valid mode." % mode)
			else:
				monome_mode(self.monome, <monome_mode_t> mode)

	property intensity:
		def __set__(self, uint intensity):
			monome_intensity(self.monome, intensity)

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

		# will raise a TypeError if rows is not iterable
		rowiter = iter(rows)

		# cython :(
		r[0] = r[1] = r[2] = r[3] = r[4] =\
			r[5] = r[6] = r[7] = 0

		try:
			for i from 0 <= i < 8:
				d = _bitmap_data(rowiter.next())
				r[i] = (<uint8_t *> &d)[0]
		except StopIteration:
			pass 

		monome_led_frame(self.monome, quadrant, r)
