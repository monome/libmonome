#!/usr/bin/env python

import time
import sys

top = "."
out = "build"

# change this stuff

APPNAME = "libmonome"
VERSION = "1.2"

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
	conf.check_cc(
			define_name="HAVE_LIBUDEV",
			mandatory=False,
			quote=0,

			execute=True,

			lib="udev",
			uselib_store="UDEV",

			msg="Checking for libudev",
			errmsg="no (will use sysfs)")

def check_liblo(conf):
	conf.check_cc(
			define_name="HAVE_LO",
			mandatory=True,
			quote=0,

			execute=True,

			lib="lo",
			uselib_store="LO",

			msg="Checking for liblo")

#
# waf stuff
#

def options(opt):
	opt.load("compiler_c")
	opt.load("cython")

	lm_opts = opt.add_option_group("libmonome options")

	lm_opts.add_option("--disable-osc", action="store_true",
			default=False, help="disable OSC/liblo support [enabled by default]")
	lm_opts.add_option("--enable-python", action="store_true",
			default=False, help="enable python bindings [disabled by default]")
	lm_opts.add_option("--enable-multilib", action="store_true",
			default=False, help="on Darwin, build libmonome as a combination 32 and 64 bit library [disabled by default]")

def configure(conf):
	# just for output prettifying
	# print() (as a function) ddoesn't work on python <2.7
	separator = lambda: sys.stdout.write("\n")

	separator()
	conf.load("compiler_c")
	conf.load("gnu_dirs")

	if conf.check_endianness() == "big":
		conf.define("LM_BIG_ENDIAN", 1)

	#
	# conf checks
	#

	separator()

	if conf.env.DEST_OS != "win32":
		check_poll(conf)
		conf.check_cc(lib='dl', uselib_store='DL', mandatory=True)

	if conf.env.DEST_OS == "linux":
		check_udev(conf)

	if not conf.options.disable_osc:
		check_liblo(conf)

	separator()

	if conf.options.enable_python:
		conf.load("python")
		conf.check_python_version(minver=(2,7,0))
		conf.check_python_headers()
		conf.load("cython")

		separator()

	#
	# setting defines, etc
	#

	if conf.options.enable_multilib:
		conf.env.ARCH = ["i386", "x86_64"]

	if conf.env.DEST_OS == "darwin":
		conf.env.append_unique("CFLAGS", ["-mmacosx-version-min=10.5"])
		conf.env.append_unique("LINKFLAGS", ["-mmacosx-version-min=10.5"])

	conf.env.append_unique("CFLAGS", ["-std=c99", "-Wall", "-Werror"])
	conf.env.PROTOCOLS = ["40h", "series", "mext"]

	conf.define("LIBDIR", conf.env.LIBDIR)
	conf.define("LIBSUFFIX", "." + conf.env.cshlib_PATTERN.rsplit(".", 1)[-1])

	conf.env.VERSION = VERSION
	conf.define("VERSION", VERSION)

	conf.write_config_header("config.h")

def build(bld):
	bld.add_post_fun(post)

	bld.install_files("${PREFIX}/include", ["public/monome.h"])
	bld.recurse("src")
	bld.recurse("utils")

	# win32 doesn't have nanosleep()
	if bld.env.DEST_OS != "win32":
		bld.recurse("examples")

	# man page
	bld(
		source="doc/monomeserial.in.1",
		target="doc/monomeserial.1",
		install_path="${PREFIX}/share/man/man1",

		features="subst",
		MDOCDATE=time.strftime("%B %d, %Y"))

	if bld.env.CYTHON:
		bld.recurse("bindings/python")

def post(pst):
	if pst.cmd == "install" and \
			pst.env.DEST_OS not in ["win32", "darwin"]:
		pst.exec_command("ldconfig")

def dist(dst):
	pats = [".git*", "**/.git*", ".travis.yml"]
	with open(".gitignore") as gitignore:
	    for l in gitignore.readlines():
	        if l[0] == "#":
	            continue

	        pats.append(l.rstrip("\n"))

	dst.excl = " ".join(pats)
