CFLAGS  := -I./private -I../public $(CFLAGS)
LDFLAGS := -L. $(LDFLAGS)

LIBMONOME = libmonome.$(LM_SUFFIX)
LMOBJS = libmonome.o rotation.o platform/$(PLATFORM).o

MONOMESERIAL = monomeserial
MSOBJS = monomeserial.o $(LIBMONOME)

all: $(LIBMONOME) $(MS_BUILD)
	cd proto; $(MAKE)

clean:
	echo "  CLEAN   src"
	rm -f *.o platform/*.o protocol/*/*.o protocol/*/*.$(LIBSUFFIX) libmonome.so $(LIBMONOME) monomeserial
	cd proto; $(MAKE) clean

install: all
	cd proto; $(MAKE) install
	$(INSTALL) -d $(LIBDIR)
	$(INSTALL) -d $(BINDIR)

	echo "  INSTALL src/$(LIBMONOME) -> $(LIBDIR)/$(LIBMONOME)"
	$(INSTALL) $(LIBMONOME) $(LIBDIR)/$(LIBMONOME)

	case $(PLATFORM) in	\
		linux*)	\
			echo "  LDCONFIG";	\
			ldconfig -n $(LIBDIR);;	\
		\
		darwin)	\
			echo "  LN      $(LIBDIR)/$(LIBMONOME) -> $(LIBDIR)/libmonome.dylib";	\
			ln -sf $(LIBDIR)/$(LIBMONOME) $(LIBDIR)/libmonome.dylib;;	\
	esac

	echo "  INSTALL src/$(MONOMESERIAL) -> $(BINDIR)/$(MONOMESERIAL)"
	$(INSTALL) $(MONOMESERIAL) $(BINDIR)/$(MONOMESERIAL)

#
# libmonome shared library
#

libmonome.so.$(VERSION): $(LMOBJS)
	echo "  LD      src/libmonome.so"
	$(LD) $(LDFLAGS) -shared -Wl,-soname,libmonome.so $(LM_LDFLAGS) -o $@ $(LMOBJS)
	ln -sf $@ libmonome.so

libmonome.$(VERSION).dylib: $(LMOBJS)
	echo "  LD      src/libmonome.dylib"
	$(LD) $(LDFLAGS) -dynamiclib -Wl,-install_name,libmonome.dylib $(LM_LDFLAGS) -o $@ $(LMOBJS)
	ln -sf $@ libmonome.dylib

#
# platform module
#

platform/$(PLATFORM).o: platform/$(PLATFORM).c
	echo "  CC      src/$@"
	$(CC) $(CFLAGS) -c $< -o $@

#
# other things
#

$(MONOMESERIAL): $(MSOBJS)
	echo "  LD      src/monomeserial"
	$(LD) $(LDFLAGS) -L. -lmonome $(LO_LDFLAGS) -o $@ $<

.c.o:
	echo "  CC      src/$@"
	$(CC) $(LO_CFLAGS) $(CFLAGS) -c $< -o $@
