PYBIND_CFLAGS += -fPIC -I../../public
PYBIND_LDFLAGS += -L../../src -lmonome

all: monome.so

clean:
	echo "  CLEAN   bindings/python"
	rm -f monome.o monome.so

mrproper:
	rm -f monome.c

dist: monome.c

monome.so: monome.o
	echo "  LD      bindings/python/$@"
	gcc -shared $< $(PYBIND_LDFLAGS) -o $@

monome.c: monome.pyx
	echo "  CYTHON  bindings/python/$@"
	cython $< -o $@

.c.o:
	echo "  CC      bindings/python/$@"
	$(CC) $(PYBIND_CFLAGS) $(CFLAGS) -c $< -o $@
