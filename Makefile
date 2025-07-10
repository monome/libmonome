LIBMONOME := build/libmonome.a


.phony: all build python-ext python-wheel clean


all: build


$(LIBMONOME):
	@mkdir -p build && cd build && \
		cmake .. && \
		cmake --build . --config Release


build: $(LIBMONOME)


python-ext: $(LIBMONOME)
	@make -C bindings/python


python-wheel: $(LIBMONOME)
	@make -C bindings/python wheel


clean:
	@rm -rf build
	@make -C bindings/python clean

