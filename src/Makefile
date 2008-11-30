CFLAGS  += -I. -I../include
LDFLAGS += -L. -lmonome -llo
objs = libmonome.o monomeserial.o simple.o test.o linux_usbfs.o platform.o

ifeq ($(PLATFORM),Darwin)
	targets = libmonome.dylib
else
	targets = libmonome.so.$(version) 
endif
targets += monomeserial linux_usbfs simple test

all: $(objs) $(targets)

examples: simple test

clean:
	echo "  CLEAN   src"
	rm -f linux_usbfs simple monomeserial test *.o *.so*

install: all
	echo "  INSTALL $(LIBDIR)/libmonome.so"
	$(INSTALL) libmonome.so.$(version) $(LIBDIR)
	ldconfig -n $(LIBDIR)

	echo "  INSTALL $(BINDIR)/monomeserial"
	install monomeserial $(BINDIR)

libmonome: libmonome.o
libmonome.so.$(version): libmonome.o platform.o
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