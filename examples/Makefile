CFLAGS  += -I../include
LDFLAGS += -L. -lmonome
OBJS     = simple.o test.o
TARGETS  = simple test

all: $(OBJS) $(TARGETS)

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