SHELL = /bin/sh

include config.mk

export CFLAGS  += -Wall -Werror -DVERSION=\"$(VERSION)\" -DLIBSUFFIX=\".$(LIBSUFFIX)\" -DLIBDIR=\"$(LIBDIR)\" -std=c99 -pedantic
export INSTALL = install

SUBDIRS = src bindings examples

.SILENT:
.SUFFIXES:
.SUFFIXES: .c .o
.PHONY: all clean mrproper distclean install test config.mk utils

all:
	cd src; $(MAKE)
	cd bindings; $(MAKE)
	cd examples; $(MAKE)

utils: all
	cd utils; $(MAKE)

clean:
	cd src; $(MAKE) clean
	cd bindings; $(MAKE) clean
	cd examples; $(MAKE) clean
	cd utils; $(MAKE) clean

mrproper: clean
	cd bindings; $(MAKE) mrproper
	cd doc; $(MAKE) mrproper

distclean: clean
	rm -f config.mk libmonome.pc

dist: mrproper
	cd bindings; $(MAKE) dist
	cd doc; $(MAKE) dist

install:
	cd public; $(MAKE) install
	cd src; $(MAKE) install
	cd bindings; $(MAKE) install
	cd doc; $(MAKE) install

	echo "  INSTALL $(PKGCONFIGDIR)/libmonome.pc"
	$(INSTALL) -d $(PKGCONFIGDIR)
	$(INSTALL) libmonome.pc $(PKGCONFIGDIR)/libmonome.pc

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
