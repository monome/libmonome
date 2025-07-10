#!/usr/bin/python3

import os
import platform
import subprocess
from pathlib import Path
from setuptools import Extension, setup

from Cython.Build import cythonize

# -----------------------------------------------------------------------------
# constants

VERSION = '0.0.1'

PLATFORM = platform.system()

PYTHON_DIR = Path(__file__).parent
BINDINGS_DIR = PYTHON_DIR.parent
ROOT_DIR = BINDINGS_DIR.parent
BUILD_DIR = ROOT_DIR / "build"
PUBLIC_DIR = ROOT_DIR / 'public'

MONOME_INCLUDE = str(PUBLIC_DIR)
MONOME_LIBS_DIR = str(BUILD_DIR)

DEFINE_MACROS = []
EXTRA_COMPILE_ARGS = []
EXTRA_LINK_ARGS = []
EXTRA_OBJECTS = []
INCLUDE_DIRS = [
    MONOME_INCLUDE,
]
LIBRARY_DIRS = [
    MONOME_LIBS_DIR,
]
LIBRARIES = []
EXTRA_OBJECTS.extend([
    f'{MONOME_LIBS_DIR}/libmonome.a',
])


if PLATFORM == 'Darwin':
    # EXTRA_LINK_ARGS.append('-mmacosx-version-min=14.7')
    pass

if PLATFORM == 'Linux':
    pass


def mk_extension(name, sources, define_macros=None):
    return Extension(
        name=name,
        sources=sources,
        define_macros=define_macros if define_macros else [],
        include_dirs=INCLUDE_DIRS,
        libraries=LIBRARIES,
        library_dirs=LIBRARY_DIRS,
        extra_objects=EXTRA_OBJECTS,
        extra_compile_args=EXTRA_COMPILE_ARGS,
        extra_link_args=EXTRA_LINK_ARGS,
    )


# ----------------------------------------------------------------------------
# COMMON SETUP CONFIG

common = {
    "name": "monome",
    "version": VERSION,
    "description": "A python extension which wraps libmonome using cython.",
    "python_requires": ">=3.8",
    # "include_package_data": True,
}


# forces cythonize in this case
subprocess.call("cythonize monome.pyx", cwd=".", shell=True)

# if not os.path.exists('MANIFEST.in'):
#     with open("MANIFEST.in", "w") as f:
#         f.write("include monome.pyx\n")


extensions = [
    mk_extension("monome", sources=[
        "monome.pyx",
    ]),
]

setup(
    **common,
    ext_modules=cythonize(
        extensions,
        compiler_directives = {
            'language_level' : '3',
            'embedsignature': False,     # default: False
            'emit_code_comments': False, # default: True
            'warn.unused': True,         # default: False
        },
    ),
)

