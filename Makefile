SHELL = /bin/sh

export project = libmonome
export version = 0.2

export CC = gcc
export LD = gcc
export CFLAGS  += -ggdb -Wall -Werror -fPIC -D_GNU_SOURCE
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

clean:
	cd src; $(MAKE) clean

install:
	cd src; $(MAKE) install
	cd include; $(MAKE) install

	echo "  INSTALL $(PKGCONFIGDIR)/libmonome.pc"
	$(INSTALL) libmonome.pc $(PKGCONFIGDIR)

test: all
	./src/test
