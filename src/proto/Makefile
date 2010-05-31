CFLAGS += -I. -I../private -I../../public

LDFLAGS = -lc -ldl -L.. -lmonome

all: $(PROTOCOLS)

clean:
	rm -f *.o *.so

install:
	$(INSTALL) -d $(LIBDIR)/monome

	for PROTOCOL in $(PROTOCOLS); do \
		echo "  INSTALL src/proto/protocol_$$PROTOCOL.$(LIBSUFFIX) -> $(LIBDIR)/monome/protocol_$$PROTOCOL.$(LIBSUFFIX)"; \
		$(INSTALL) protocol_$$PROTOCOL.$(LIBSUFFIX) $(LIBDIR)/monome; \
	done

# just a little hack :)
series: protocol_series.$(LIBSUFFIX)
	true
40h: protocol_40h.$(LIBSUFFIX)
	true
osc: protocol_osc.$(LIBSUFFIX)
	true

protocol_series.so: series.o
	echo "  LD      src/proto/$@"
	$(LD) -shared -Wl,-soname,$@ $(LDFLAGS) -o $@ $<

protocol_series.dylib: series.o
	echo "  LD      src/proto/$@"
	$(LD) -dynamiclib -Wl,-dylib_install_name,$@ $(LDFLAGS) -o $@ $<

protocol_40h.so: 40h.o
	echo "  LD      src/proto/$@"
	$(LD) -shared -Wl,-soname,$@ $(LDFLAGS) -o $@ $<

protocol_40h.dylib: 40h.o
	echo "  LD      src/proto/$@"
	$(LD) -dynamiclib -Wl,-dylib_install_name,$@ $(LDFLAGS) -o $@ $<

protocol_osc.so: osc.o
	echo "  LD      src/proto/$@"
	$(LD) -shared -Wl,-soname,$@ $(LDFLAGS) -llo -o $@ $<

protocol_osc.dylib: osc.o
	echo "  LD      src/proto/$@"
	$(LD) -dynamiclib -Wl,-dylib_install_name,$@ $(LDFLAGS) -llo -o $@ $<

.c.o:
	echo "  CC      src/proto/$@"
	$(CC) $(CFLAGS) -c $< -o $@
