CFLAGS  := -I../src/private -I../public $(CFLAGS)
LDFLAGS := -L../src $(LDFLAGS)

MKSETGRIDS = mk-set-grids
MKSGOBJS = mk-set-grids.o

all: $(MKSETGRIDS)

clean:
	echo "  CLEAN   utils"
	rm -f *.o mk-set-grids

$(MKSETGRIDS): $(MKSGOBJS)
	echo "  LD      utils/mk-set-grids"
	$(LD) $(LDFLAGS) -L. -lmonome -o $@ $<

.c.o:
	echo "  CC      utils/$@"
	$(CC) $(LO_CFLAGS) $(CFLAGS) -c $< -o $@
