#!/usr/bin/env python

top = "."
out = "build"

#
# dep checking functions
#

def check_poll(ctx):
	# borrowed from glib's poll test

	code = """
		#include <stdlib.h>
		#include <poll.h>
		
		int main(int argc, char **argv) {
		    struct pollfd fds[1];
		
		    fds[0].fd = open("/dev/null", 1);
		    fds[0].events = POLLIN;
		
		    if( poll(fds, 1, 0) < 0 || fds[0].revents & POLLNVAL )
		        exit(1);
		    exit(0);
		}"""

	ctx.check_cc(
			define_name="HAVE_WORKING_POLL",
			mandatory=False,
			quote=0,

			execute=True,

			fragment=code,

			msg="Checking for working poll()",
			errmsg="no (will use select())")

def check_udev(ctx):
	code = """
		#include <libudev.h>
		
		int main(int argc, char **argv) {
		    struct udev *udev = udev_new();
		    udev_unref(udev);
		
		    return 0;
		}"""

	ctx.check_cc(
			define_name="HAVE_LIBUDEV",
			mandatory=False,
			quote=0,

			execute=True,

			lib="udev",
			uselib_store="UDEV",
			fragment=code,

			msg="Checking for libudev",
			errmsg="no (will use sysfs)")

def check_liblo(ctx):
	code = """
		#include <lo/lo.h>
		
		int main(int argc, char **argv) {
		    lo_address a = lo_address_new(NULL, "42424");
		    lo_address_free(a);
		    return 0;
		}"""

	r = ctx.check_cc(
			define_name="HAVE_LO",
			mandatory=True,
			quote=0,

			execute=True,

			lib="lo",
			uselib_store="LO",
			fragment=code,

			msg="Checking for liblo")

#
# waf stuff
#

def options(ctx):
	ctx.load("compiler_c")

	lm_opts = ctx.add_option_group("libmonome options")
	lm_opts.add_option("--disable-osc", action="store_true",
			default=False, help="disable OSC/liblo support [enabled by default]")

def configure(ctx):
	ctx.load("compiler_c")

	print("")
	check_poll(ctx)
	check_udev(ctx)

	if not ctx.options.disable_osc:
		check_liblo(ctx)

	print("")
