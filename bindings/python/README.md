# monome

A python extension which wraps libmonome using cython.

## Requirements to build:

- `setuptools`

- `cython`


## Building

(From root of `libmonome` project)

Building extension 

```sh
make python-ext
```

Building wheel:

```sh
make python-wheel
```

The wheel will be found 

