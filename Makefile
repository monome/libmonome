LIBMONOME := build/libmonome.a


.phony: all build python-ext clean reset


all: build


$(LIBMONOME):
	@mkdir -p build && cd build && \
		cmake .. && \
		cmake --build . --config Release

build: $(LIBMONOME)


python-ext: $(LIBMONOME)
	@uv build


clean:
	@rm -rf build dist


reset: clean
	@rm -rf .venv
