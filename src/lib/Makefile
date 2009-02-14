CFLAGS  += -I./include -I../include
LDFLAGS += 
OBJS = libmonome.o platform.o protocol.o

PROTO = series

ifeq ($(PLATFORM),Darwin)
	TARGETS = libmonome.dylib
else
	TARGETS = libmonome.so.$(VERSION)
endif

all: $(OBJS) $(TARGETS)

clean:
	echo "  CLEAN   src/lib"
	rm -f linux_usbfs simple monomeserial test *.o *.so*

install: all
	echo "  INSTALL src/lib/libmonome.so.$(VERSION) -> $(LIBDIR)/libmonome.so$(VERSION)"
	$(INSTALL) libmonome.so.$(VERSION) $(LIBDIR)
ifneq ($(PLATFORM),Darwin)
	ldconfig -n $(LIBDIR)
endif

libmonome: libmonome.o
libmonome.so.$(VERSION): libmonome.o platform.o protocol.o
	echo "  LD      src/lib/libmonome.so"
	$(LD) -shared -Wl,-soname,libmonome.so -lc -o $@ $^
	ln -sf $@ libmonome.so

libmonome.dylib: libmonome.o platform.o
	echo "  LD      src/lib/libmonome.so"
	$(LD) -dynamiclib -Wl,-dylib_install_name,libmonome.dylib -lc -o $@ $^

platform.o:
	echo "  CC      src/lib/$@"
ifeq ($(PLATFORM),Darwin)
	$(CC) $(CFLAGS) -c platform/darwin.c -o $@
else
	$(CC) $(CFLAGS) -c platform/linux.c -o $@
endif

protocol.o: protocol/$(PROTO)/protocol_$(PROTO).c
	echo "  CC      src/lib/$@"
	$(CC) $(CFLAGS) -c $< -o $@

%: %.o
	echo "  LD      src/lib/$@"
	$(LD) $(LDFLAGS) $< -o $@

%.o: %.c
	echo "  CC      src/lib/$@"
	$(CC) $(CFLAGS) -c $< -o $@