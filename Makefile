SHELL = /bin/sh

export PROJECT = libmonome
export VERSION = 0.2

export CC = gcc
export LD = gcc
export CFLAGS  += -ggdb -Wall -Werror -fPIC
export LDFLAGS += -ggdb
export INSTALL = install

export PREFIX = /usr
export BINDIR = $(PREFIX)/bin
export LIBDIR = $(PREFIX)/lib
export INCDIR = $(PREFIX)/include
export PKGCONFIGDIR = $(LIBDIR)/pkgconfig

export PLATFORM = $(shell uname -s)

.SILENT:
.SUFFIXES:
.SUFFIXES: .c .o
.PHONY: all clean install

all:
	cd src; $(MAKE)
	cd examples; $(MAKE)

clean:
	cd src; $(MAKE) clean
	cd examples; $(MAKE) clean

install:
	cd src; $(MAKE) install

	echo "  INSTALL $(PKGCONFIGDIR)/libmonome.pc"
	$(INSTALL) libmonome.pc $(PKGCONFIGDIR)

test: all
	./src/test
