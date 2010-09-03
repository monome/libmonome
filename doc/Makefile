TARGETS = monomeserial.1

all: $(TARGETS)

install: $(TARGETS)
	if [ -n "$(MANDIR)" ]; then \
		mkdir -p $(MANDIR)/man1 \
		echo "  INSTALL $(MANDIR)/man1/monomeserial.1"; \
		$(INSTALL) -d $(MANDIR); \
		$(INSTALL) monomeserial.1 $(MANDIR)/man1/monomeserial.1; \
	fi

dist: $(TARGETS)

mrproper:
	rm -f monomeserial.1

monomeserial.1: monomeserial.in.1
	sed -e "s/\$Mdocdate\$$/`date +"%B %-d, %Y"`/g" < $< > $@
