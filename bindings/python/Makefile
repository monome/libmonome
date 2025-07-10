.phony: all build inplace wheel dist clean

all: build

build:
	@python3 setup.py build_ext

inplace:
	@python3 setup.py build_ext --inplace
	
wheel:
	@python3 setup.py bdist_wheel

dist: wheel

clean:
	@rm -rf build dist monome.c monome.*.so *.egg-info
