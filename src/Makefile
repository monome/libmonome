CFLAGS  := -I./private -I../public $(CFLAGS)
LDFLAGS := -L. $(LDFLAGS)

# haaaack
PLATFORM_linux_libudev_OBJS = platform/linux_libudev.o platform/linux.o platform/posix.o
PLATFORM_linux_sysfs_OBJS = platform/linux_sysfs.o platform/linux.o platform/posix.o
PLATFORM_darwin_OBJS = platform/darwin.o platform/posix.o
PLATFORM_windows_OBJS = platform/windows.o

LIBMONOME = libmonome.$(LM_SUFFIX)
LMOBJS = libmonome.o rotation.o $(PLATFORM_$(PLATFORM)_OBJS)

MONOMESERIAL = monomeserial
MSOBJS = monomeserial.o $(LIBMONOME)

all: $(LIBMONOME) $(MS_BUILD)
	cd proto; $(MAKE)

clean:
	echo "  CLEAN   src"
	rm -f *.o platform/*.o protocol/*.o protocol/*.$(LIBSUFFIX) libmonome.$(LIBSUFFIX) $(LIBMONOME) monomeserial
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

libmonome.so.$(VERSION): $(LMOBJS)
	echo "  LD      src/libmonome.so"
	$(LD) $(LDFLAGS) -shared -Wl,-soname,libmonome.so $(LM_LDFLAGS) -o $@ $(LMOBJS)
	ln -sf $@ libmonome.so

libmonome.$(VERSION).dylib: $(LMOBJS)
	echo "  LD      src/libmonome.dylib"
	$(LD) $(LDFLAGS) -dynamiclib -Wl,-install_name,libmonome.dylib $(LM_LDFLAGS) -o $@ $(LMOBJS)
	ln -sf $@ libmonome.dylib

libmonome.dll: $(LMOBJS)
	echo "  LD      src/$@"
	$(LD) $(LDFLAGS) $(LM_LDFLAGS) -shared $(LMOBJS) -o $@

$(MONOMESERIAL): $(MSOBJS)
	echo "  LD      src/monomeserial"
	$(LD) $(LDFLAGS) -L. -lmonome $(LO_LDFLAGS) -o $@ $<

.c.o:
	echo "  CC      src/$@"
	$(CC) $(LO_CFLAGS) $(CFLAGS) -o $@ -c $<
