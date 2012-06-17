#!/usr/bin/env python

top = "."
out = "build"

# change this stuff

project = "libmonome"
version = "1.1"

#
# dep checking functions
#

def check_poll(conf):
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

	conf.check_cc(
			define_name="HAVE_WORKING_POLL",
			mandatory=False,
			quote=0,

			execute=True,

			fragment=code,

			msg="Checking for working poll()",
			errmsg="no (will use select())")

def check_udev(conf):
	code = """
		#include <libudev.h>

		int main(int argc, char **argv) {
		    struct udev *udev = udev_new();
		    udev_unref(udev);

		    return 0;
		}"""

	conf.check_cc(
			define_name="HAVE_LIBUDEV",
			mandatory=False,
			quote=0,

			execute=True,

			lib="udev",
			uselib_store="UDEV",
			fragment=code,

			msg="Checking for libudev",
			errmsg="no (will use sysfs)")

def check_liblo(conf):
	code = """
		#include <lo/lo.h>

		int main(int argc, char **argv) {
		    lo_address a = lo_address_new(NULL, "42424");
		    lo_address_free(a);
		    return 0;
		}"""

	conf.check_cc(
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

def options(opt):
	opt.load("compiler_c")

	lm_opts = opt.add_option_group("libmonome options")
	lm_opts.add_option("--disable-osc", action="store_true",
			default=False, help="disable OSC/liblo support [enabled by default]")

def configure(conf):
	conf.load("compiler_c")

	conf.check_library()
	if conf.check_endianness() == "big":
		conf.define("LM_BIG_ENDIAN", 1)

	#
	# conf checks
	#

	print("")

	check_poll(conf)
	check_udev(conf)

	if not conf.options.disable_osc:
		check_liblo(conf)

	print("")

	#
	# setting defines, etc
	#

	conf.env.append_unique("CFLAGS", ["-std=c99", "-Wall", "-Werror"])
	conf.env.PROTOCOLS = ["40h", "series", "mext"]

	conf.define("LIBDIR", conf.env.LIBDIR)
	conf.define("LIBSUFFIX", "." + conf.env.cshlib_PATTERN.rsplit(".", 1)[-1])

	conf.env.VERSION = version
	conf.define("VERSION", version)

	conf.write_config_header("config.h")

def build(bld):
	bld.install_files("${PREFIX}/include", ["public/monome.h"])
	bld.recurse("src")
