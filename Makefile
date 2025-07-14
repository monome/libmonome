LIBMONOME := build/libmonome.a


.phony: all build python-cmake python clean


all: build


$(LIBMONOME):
	@mkdir -p build && cd build && \
		cmake .. && \
		cmake --build . --config Release

build: $(LIBMONOME)

python-cmake: $(LIBMONOME)
	 @mkdir -p build && cd build && \
		cmake .. -DBUILD_PYTHON_EXTENSION=ON && \
		cmake --build . --config Release

python: $(LIBMONOME)
	@python3 bindings/python/setup.py \
		build --build-base build/cython \
	    egg_info --egg-base build/cython \
	    sdist --dist-dir build/cython \
	    bdist_wheel --dist-dir build/cython

clean:
	@rm -rf build
