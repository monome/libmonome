CFLAGS += -I. -I../include

LIBMONOME = libmonome.$(LMSUFFIX)
LMOBJS = libmonome.o platform.o rotation.o

MONOMESERIAL = monomeserial
MSOBJS = monomeserial.o

all: $(LIBMONOME) $(MS_BUILD)
	cd proto; $(MAKE)
	cd testing; $(MAKE)

clean:
	echo "  CLEAN   src"
	rm -f *.o platform/*.o protocol/*/*.o protocol/*/*.$(LIBSUFFIX) libmonome.so $(LIBMONOME) monomeserial
	cd proto; $(MAKE) clean
	cd testing; $(MAKE) clean

install: all
	cd proto; $(MAKE) install

	echo "  INSTALL src/$(LIBMONOME) -> $(LIBDIR)/$(LIBMONOME)"
	$(INSTALL) $(LIBMONOME) $(LIBDIR)

	if `which ldconfig > /dev/null`; then \
		echo "  LDCONFIG"; \
		ldconfig -n $(LIBDIR); \
	fi

libmonome.so.$(VERSION): $(LMOBJS)
	echo "  LD      src/libmonome.so"
	$(LD) -shared -Wl,-soname,libmonome.so -lc -ldl -o $@ $(LMOBJS)
	ln -sf $@ libmonome.so

libmonome.dylib: $(LMOBJS)
	echo "  LD      src/libmonome.dylib"
	$(LD) -dynamiclib -Wl,-dylib_install_name,libmonome.dylib -lc -ldl -o $@ $(LMOBJS)

platform.o: platform/$(PLATFORM).c
	echo "  CC      src/$@"
	$(CC) $(CFLAGS) -c $< -o $@

$(MONOMESERIAL): $(MSOBJS)
	echo "  LD      src/monomeserial"
	$(LD) -L. -lmonome -llo -o $@ $<

.c.o:
	echo "  CC      src/$@"
	$(CC) $(CFLAGS) -c $< -o $@
