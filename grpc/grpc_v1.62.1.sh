#!/bin/bash

source check_cmake.sh

clone() {
	echo "=== clone grpc ${1} ;  ==="

	# checkout grpc
	git clone -b v1.62.1 --depth 1 --recursive https://github.com/grpc/grpc.git ${2}
}

build_grpc() {
	echo "=== build_grpc ==="
	mkdir -p grpc/build
	pushd grpc/build
	
	if [[ "${HOST}" = "Linux"  && "${TARGET}" == "linux64" ]];then
		cmake -DgRPC_INSTALL=ON \
				-DCMAKE_BUILD_TYPE=Release \
				-DgRPC_BUILD_TESTS=OFF \
				-DCMAKE_INSTALL_PREFIX=${RUNTIME}/grpc-v1.62.1 \
		..
		time make VERBOSE=1 -j16
		time make install
	elif [[ "${HOST}" = "Linux"  && "${TARGET}" == "win64" ]];then
		echo "NO LONGER KNOW HOW TO BUILD GRPC VIA MINGW_W64 ON DEBIAN ANYMORE."
	fi
	
	popd
}

install() {
	echo ====grpc build process start====

	# check cmake version need greate and equal than v3.16.1
	check_cmake_version
	echo `pwd`
	
	# clone
	if [ ! -d "grpc" ];then
		time clone "v1.62.1" "grpc"
	fi
	
	# build host grpc to /usr/local
	if [ ! -f "${RUNTIME}/grpc-v1.62.1/bin/grpc_cpp_plugin" ];then
		build_grpc
	fi
}

$@
