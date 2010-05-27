CFLAGS  += -I../libmonome
LDFLAGS += -L../src -lmonome
TARGETS  = simple test life

all: $(TARGETS)

simple: simple.o
test: test.o
life: life.o

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
