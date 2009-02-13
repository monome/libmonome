CFLAGS  += -I./include -I../include
LDFLAGS += -L. -lmonome -llo
OBJS = libmonome.o monomeserial.o linux_usbfs.o platform.o

ifeq ($(PLATFORM),Darwin)
	TARGETS = libmonome.dylib
else
	TARGETS = libmonome.so.$(VERSION)
endif
TARGETS += monomeserial linux_usbfs

all: $(OBJS) $(TARGETS)

clean:
	echo "  CLEAN   src"
	rm -f linux_usbfs simple monomeserial test *.o *.so*

install: all
	echo "  INSTALL $(LIBDIR)/libmonome.so"
	$(INSTALL) libmonome.so.$(VERSION) $(LIBDIR)
	ldconfig -n $(LIBDIR)

	echo "  INSTALL $(BINDIR)/monomeserial"
	install monomeserial $(BINDIR)

libmonome: libmonome.o
libmonome.so.$(VERSION): libmonome.o platform.o
	echo "  LD      src/libmonome.so"
	$(LD) -shared -Wl,-soname,libmonome.so -lc -o $@ $^
	ln -sf $@ libmonome.so

libmonome.dylib: libmonome.o platform.o
	echo "  LD      src/libmonome.so"
	$(LD) -dynamiclib -Wl,-dylib_install_name,libmonome.dylib -lc -o $@ $^

platform.o:
ifeq ($(PLATFORM),Linux)
	echo "  CC      src/$@"
	$(CC) $(CFLAGS) -c platforms/linux.c -o $@
else
ifeq ($(PLATFORM),Darwin)
	echo "  CC      src/$@"
	$(CC) $(CFLAGS) -c platforms/darwin.c -o $@
else
	echo "  CC      src/$@ (warning: compiled with dummy platform, libmonome will not actually do anything)"
	$(CC) $(CFLAGS) -c platforms/dummy.c -o $@
endif
endif

%: %.o
	echo "  LD      src/$@"
	$(LD) $(LDFLAGS) $< -o $@

%.o: %.c
	echo "  CC      src/$@"
	$(CC) $(CFLAGS) -c $< -o $@