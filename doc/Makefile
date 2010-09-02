TARGETS = monomeserial.1

all: $(TARGETS)

monomeserial.1: monomeserial.in.1
	sed -e "s/\$Mdocdate\$$/`date +"%B %-d, %Y"`/g" < $< > $@
