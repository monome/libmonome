CFLAGS += -I.. -I../../include
LDFLAGS = 

all: linux_sysfs linux_libudev

clean:
	echo "  CLEAN   src/testing"
	rm -f linux_sysfs linux_libudev *.o

linux_sysfs: linux_sysfs.o
	echo "  LD      src/testing/$@"
	$(LD) $(LDFLAGS) $< -o $@

linux_libudev: linux_libudev.o
	echo "  LD      src/testing/$@"
	$(LD) $(LDFLAGS) -ludev $< -o $@

.c.o:
	echo "  CC      src/testing/$@"
	$(CC) $(CFLAGS) -c $< -o $@
