CFLAGS  := -I./private -I../public $(CFLAGS)
LDFLAGS := -L. $(LDFLAGS)

LIBMONOME = libmonome.$(LM_SUFFIX)
LMOBJS = libmonome.o platform.o rotation.o

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

	if `which ldconfig > /dev/null`; then \
		echo "  LDCONFIG"; \
		ldconfig -n $(LIBDIR); \
	fi

	echo "  INSTALL src/$(MONOMESERIAL) -> $(BINDIR)/$(MONOMESERIAL)"
	$(INSTALL) $(MONOMESERIAL) $(BINDIR)/$(MONOMESERIAL)

libmonome.so.$(VERSION): $(LMOBJS)
	echo "  LD      src/libmonome.so"
	$(LD) $(LDFLAGS) -shared -Wl,-soname,libmonome.so $(LM_LDFLAGS) -o $@ $(LMOBJS)
	ln -sf $@ libmonome.so

libmonome.dylib: $(LMOBJS)
	echo "  LD      src/libmonome.dylib"
	$(LD) $(LDFLAGS) -dynamiclib -Wl,-install_name,libmonome.dylib $(LM_LDFLAGS) -o $@ $(LMOBJS)

platform.o: platform/$(PLATFORM).c
	echo "  CC      src/$@"
	$(CC) $(CFLAGS) -c $< -o $@

$(MONOMESERIAL): $(MSOBJS)
	echo "  LD      src/monomeserial"
	$(LD) $(LDFLAGS) -L. -lmonome $(LO_LDFLAGS) -o $@ $<

.c.o:
	echo "  CC      src/$@"
	$(CC) $(LO_CFLAGS) $(CFLAGS) -c $< -o $@
