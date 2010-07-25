PYBIND_CFLAGS += -fPIC -I../../public
PYBIND_LDFLAGS += -L../../src -lmonome
MONOME_BINDING = monome.$(LIBSUFFIX)

all: $(MONOME_BINDING)

install: all
	$(INSTALL) -d $(PYTHON_INSTALL_DIR)

	# python doesn't recognize .dylib as a valid extension for modules,
	# so we install it as a .so regardless of the OS.
	echo "  INSTALL bindings/python/monome.$(LIBSUFFIX) -> $(PYTHON_INSTALL_DIR)/monome.so"
	$(INSTALL) monome.so $(PYTHON_INSTALL_DIR)/monome.so

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

monome.c: monome.pyx
	echo "  CYTHON  bindings/python/$@"
	cython $< -o $@

.c.o:
	echo "  CC      bindings/python/$@"
	$(CC) $(PYBIND_CFLAGS) $(CFLAGS) -c $< -o $@
