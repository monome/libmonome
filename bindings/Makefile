.PHONY: $(BINDINGS) all clean mrproper dist instal

all: $(BINDINGS)

python:
	cd python; $(MAKE)


install:
	for DIR in $(BINDINGS); do \
		sh -c "cd $$DIR && $(MAKE) install"; \
	done

clean:
	cd python; $(MAKE) clean

mrproper:
	cd python; $(MAKE) mrproper

dist:
	cd python; $(MAKE) dist
