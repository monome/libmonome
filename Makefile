SHELL = /bin/sh

include config.mk

export PROJECT = libmonome

export CFLAGS  += -ggdb -Wall -Werror -fPIC -DVERSION=\"$(VERSION)\" -DLIBSUFFIX=\".$(LIBSUFFIX)\" -DLIBDIR=\"$(LIBDIR)\"
export LDFLAGS += -ggdb
export INSTALL = install

export BINDIR = $(PREFIX)/bin
export LIBDIR = $(PREFIX)/lib
export INCDIR = $(PREFIX)/include
export PKGCONFIGDIR = $(LIBDIR)/pkgconfig

.SILENT:
.SUFFIXES:
.SUFFIXES: .c .o
.PHONY: all clean distclean install test config.mk

all:
	cd src; $(MAKE)
	cd examples; $(MAKE)

clean:
	cd src; $(MAKE) clean
	cd examples; $(MAKE) clean

distclean: clean
	rm -f config.mk libmonome.pc

install:
	cd public; $(MAKE) install
	cd src; $(MAKE) install

	echo "  INSTALL $(PKGCONFIGDIR)/libmonome.pc"
	$(INSTALL) libmonome.pc $(PKGCONFIGDIR)

config.mk:
	if [ ! -f config.mk ]; then \
	echo ;\
	echo "  _                " ;\
	echo " | |__   ___ _   _ " ;\
	echo " |  _ \ / _ \ | | |    you need to run " ;\
	echo " | | | |  __/ |_| |  ./configure first!" ;\
	echo " |_| |_|\___|\__, |" ;\
	echo "             |___/ " ;\
	echo ;\
	fi


test: all
	./src/test
