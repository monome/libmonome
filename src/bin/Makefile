CFLAGS  += -I../include
LDFLAGS += -L../lib -lmonome -llo -ludev
OBJS = monomeserial.o linux_sysfs.o linux_libudev.o
TARGETS = monomeserial linux_sysfs linux_libudev

all: $(OBJS) $(TARGETS)

linux_libudev: linux_libudev.o
	echo "  LD      src/bin/$@"
	$(LD) $(LDFLAGS) -ludev $< -o $@

clean:
	echo "  CLEAN   src/bin"
	rm -f $(OBJS) $(TARGETS)

install: all
	echo "  INSTALL src/bin/monomeserial -> $(BINDIR)/monomeserial"
	install monomeserial $(BINDIR)

%: %.o
	echo "  LD      src/bin/$@"
	$(LD) $(LDFLAGS) $< -o $@

%.o: %.c
	echo "  CC      src/bin/$@"
	$(CC) $(CFLAGS) -c $< -o $@
