CFLAGS  += -I../public
LDFLAGS := -L../src -lmonome $(LDFLAGS)
TARGETS  = simple test life torture test_arc

all: $(TARGETS)

simple: simple.o
test: test.o
test_arc: test_arc.o
life: life.o
torture: torture.o

clean:
	echo "  CLEAN   examples"
	rm -f $(TARGETS) *.o

install:
	echo -n ""

%: %.o
	echo "  LD      examples/$@"
	$(LD) $(LDFLAGS) $< -o $@

%.o: %.c
	echo "  CC      examples/$@"
	$(CC) $(CFLAGS) -c $< -o $@
