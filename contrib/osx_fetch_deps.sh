#!/bin/bash

get_cython() {
	REVISION="0.12.1"
	try_command "curl -O http://www.cython.org/release/Cython-$REVISION.tar.gz"
	try_command "tar xzvf Cython-$REVISION.tar.gz"
	export CDIR=`pwd`
	cd "Cython-$REVISION"
	try_command "python setup.py install"
	cd $CDIR
	rm -rf "Cython-$REVISION"
	rm "Cython-$REVISION.tar.gz"

	return 0;
}
