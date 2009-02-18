ifeq ($(PLATFORM),Darwin)
	LIBSUFFIX = dylib
	LIBMONOME = libmonome.$(LIBSUFFIX)
else
	LIBSUFFIX = so
	LIBMONOME = libmonome.$(LIBSUFFIX).$(VERSION)
endif

CFLAGS  += -I../include -DLIBSUFFIX=\".$(LIBSUFFIX)\" -DLIBDIR=\"$(LIBDIR)\"
LDFLAGS += 

PROTOCOLS = $(foreach protocol,$(notdir $(wildcard protocol/*)),protocol/$(protocol)/protocol_$(protocol))
OBJS := libmonome.o platform.o $(addsuffix .o,$(PROTOCOLS))
PROTOCOL_LIBS = $(addsuffix .$(LIBSUFFIX),$(PROTOCOLS))
TARGETS = $(LIBMONOME)

all: $(PROTOCOL_LIBS) $(OBJS) $(TARGETS)

clean:
	echo "  CLEAN   src/lib"
	rm -f $(TARGETS) $(OBJS) $(PROTOCOL_LIBS) libmonome.so

install: all
	$(INSTALL) -d $(LIBDIR)/monome

	for PROTOCOL in $(PROTOCOL_LIBS); do \
		echo "  INSTALL src/lib/$$PROTOCOL -> $(LIBDIR)/monome/`basename $$PROTOCOL`"; \
		$(INSTALL) $$PROTOCOL $(LIBDIR)/monome; \
	done

	echo "  INSTALL src/lib/$(LIBMONOME) -> $(LIBDIR)/$(LIBMONOME)"
	$(INSTALL) $(LIBMONOME) $(LIBDIR)
ifneq ($(PLATFORM),Darwin)
	echo "  LDCONFIG"
	ldconfig -n $(LIBDIR)
endif

libmonome.so.$(VERSION): libmonome.o platform.o
	echo "  LD      src/lib/libmonome.so"
	$(LD) -shared -Wl,-soname,libmonome.so -lc -ldl -o $@ $^
	ln -sf $@ libmonome.so

libmonome.dylib: libmonome.o platform.o
	echo "  LD      src/lib/libmonome.so"
	$(LD) -dynamiclib -Wl,-dylib_install_name,libmonome.dylib -lc -ldl -o $@ $^

platform.o:
	echo "  CC      src/lib/$@"
ifeq ($(PLATFORM),Darwin)
	$(CC) $(CFLAGS) -c platform/darwin.c -o $@
else
	$(CC) $(CFLAGS) -c platform/linux.c -o $@
endif

%.so: %.o
	echo "  LD      src/lib/$@"
	$(LD) -shared -Wl,-soname,$@ -lc -o $@ $<

%.dylib: %.o
	echo "  LD      src/lib/$@"
	$(LD) -dynamiclib -Wl,-dylib_install_name,$@ -lc -o $@ $^

%: %.o
	echo "  LD      src/lib/$@"
	$(LD) $(LDFLAGS) $< -o $@

%.o: %.c
	echo "  CC      src/lib/$@"
	$(CC) $(CFLAGS) -c $< -o $@