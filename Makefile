LIBMONOME := build/libmonome.a


.phony: all build python-ext clean


all: build


$(LIBMONOME):
	@mkdir -p build && cd build && \
		cmake .. && \
		cmake --build . --config Release

build: $(LIBMONOME)


python-ext: $(LIBMONOME)
	@mkdir -p build && cd build && \
		cmake .. -DBUILD_PYTHON_EXTENSION=ON && \
		cmake --build . --config Release


clean:
	@rm -rf build
