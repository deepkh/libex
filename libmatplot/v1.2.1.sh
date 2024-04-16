#!/bin/bash

build_libmatplot_v1.2.1() {
	echo "=== build_grpc ==="
	mkdir -p matplotplusplus-1.2.1/build
	pushd matplotplusplus-1.2.1/build
	
	if [[ "${HOST}" = "Linux" ]];then
		cmake -DMATPLOTPP_BUILD_SHARED_LIBS=ON \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
        -DMATPLOTPP_BUILD_EXAMPLES=OFF \
        -DMATPLOTPP_BUILD_TESTS=OFF \
				-DCMAKE_INSTALL_PREFIX=${RUNTIME} \
		..
		time make VERBOSE=1 -j
		time make install
	#elif [[ "${HOST}" = "Linux"  && "${TARGET}" == "win64" ]];then
	else
		echo "NO LONGER KNOW HOW TO BUILD GRPC VIA MINGW_W64 ON DEBIAN ANYMORE."
	fi
	
	popd
}

###
### dependencies:
### sudo apt install libpng-dev libpng++-dev libjpeg-dev libtiff-dev
### sudo apt install gnuplot-nox
###
### export GNUTERM=dumb  before running the linked app
###
install() {
	echo ====grpc build process start====

	echo `pwd`
	
	# clone
	if [ ! -d "matplotplusplus-1.2.1" ];then
		wget https://github.com/alandefreitas/matplotplusplus/archive/refs/tags/v1.2.1.tar.gz
		tar -zxvpf v1.2.1.tar.gz
	fi
	
	# build host grpc to /usr/local
	if [ ! -f "${RUNTIME}/lib/libmatplot.a" ];then
		build_libmatplot_v1.2.1
	fi
}

$@
