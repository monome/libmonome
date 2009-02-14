CFLAGS  += -I../include
LDFLAGS += -L../lib -lmonome -llo
OBJS = monomeserial.o linux_usbfs.o
TARGETS = monomeserial linux_usbfs

all: $(OBJS) $(TARGETS)

clean:
	echo "  CLEAN   src/bin"
	rm -f linux_usbfs monomeserial *.o

install: all
	echo "  INSTALL src/bin/monomeserial -> $(BINDIR)/monomeserial"
	install monomeserial $(BINDIR)

%: %.o
	echo "  LD      src/bin/$@"
	$(LD) $(LDFLAGS) $< -o $@

%.o: %.c
	echo "  CC      src/bin/$@"
	$(CC) $(CFLAGS) -c $< -o $@