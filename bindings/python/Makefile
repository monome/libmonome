PYBIND_CFLAGS += -fPIC -I../../public
PYBIND_LDFLAGS += -L../../src -lmonome

all: monome.$(LIBSUFFIX)

clean:
	echo "  CLEAN   bindings/python"
	rm -f monome.o monome.so monome.$(LIBSUFFIX)

mrproper:
	rm -f monome.c

dist: monome.c

monome.so: monome.o
	echo "  LD      bindings/python/$@"
	$(LD) $(LDFLAGS) -shared $(PYBIND_LDFLAGS) -o $@ $<

monome.dylib: monome.o
	echo "  LD      bindings/python/$@"
	$(LD) $(LDFLAGS) -dynamiclib -Wl,-dylib_install_name,monome.dylib $(PYBIND_LDFLAGS) -o $@ $<

	# evidently, python doesn't recognize .dylib as a valid extension option,
	# but .so is just fine and dandy.
	ln -s monome.dylib monome.so

monome.c: monome.pyx
	echo "  CYTHON  bindings/python/$@"
	cython $< -o $@

.c.o:
	echo "  CC      bindings/python/$@"
	$(CC) $(PYBIND_CFLAGS) $(CFLAGS) -c $< -o $@
