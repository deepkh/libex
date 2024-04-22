#!/bin/bash

source check_cmake.sh

clone() {
	echo "=== clone grpc ${1} ;  ==="

	# checkout grpc
	git clone -b ${1} --depth 1 --recursive https://github.com/grpc/grpc.git ${1}

    # check to safe version
    pushd ${1}/third_party/cares/cares
    git fetch --all
    git checkout -b 1.18.1 tags/cares-1_18_1
    popd
}

build_grpc_linux64() {

	if [ ! -f ${ROOT}/runtime.linux64/grpc-${1}/bin/grpc_cpp_plugin ];then
    	echo "=== build_grpc_linux64 ${1} ==="

        mkdir -p ${ROOT}/runtime.linux64/ 1> /dev/null 2>/dev/null
    	mkdir -p ${1}/build_host
    	pushd ${1}/build_host

        cmake -DgRPC_INSTALL=ON \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_CXX_STANDARD=17 \
            -DgRPC_BUILD_TESTS=OFF \
            -DCMAKE_INSTALL_PREFIX=${ROOT}/runtime.linux64/grpc-${1} \
        ..
        time make VERBOSE=1 -j12
        time make install

        popd
	fi
}

build_grpc_aarch64() {

	if [ ! -f ${RUNTIME}/grpc-${1}/bin/grpc_cpp_plugin ];then
    	echo "=== build_grpc_linux_aarch64 ${1} ==="

    	mkdir -p ${1}/build_aarch64
    	pushd ${1}/build_aarch64

        cmake -DgRPC_INSTALL=ON \
            -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_CXX_STANDARD=17 \
            -DgRPC_BUILD_TESTS=OFF \
            -DCMAKE_INSTALL_PREFIX=${RUNTIME}/grpc-${1}/ \
        ..
        time make VERBOSE=1 -j12
        time make install

        popd
	fi
}


build_grpc_mingw_w64() {

	if [ ! -f ${RUNTIME}/grpc-${1}/bin/grpc_cpp_plugin.exe ];then
    	echo "=== build_grpc_mingw-w64 ${1} ==="

    	mkdir -p ${1}/build_mingw-w64
    	pushd ${1}/build_mingw-w64

cat > toolchain.cmake <<'EOT'
SET(CMAKE_SYSTEM_NAME Windows)
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
SET(CMAKE_SYSTEM_PROCESSOR x86_64)
#set(CMAKE_SYSROOT /usr/x86_64-w64-mingw32)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc-posix)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++-posix)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX} /usr/lib/gcc/${TOOLCHAIN_PREFIX}/6.3-posix)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
set(CMAKE_CXX_STANDARD 17)
EOT

        cmake -DgRPC_INSTALL=ON \
		-DCMAKE_BUILD_TYPE=Release \
		-DgRPC_BUILD_TESTS=OFF \
		-DCMAKE_INSTALL_PREFIX=${RUNTIME}/grpc-${1} \
		-DCMAKE_TOOLCHAIN_FILE=`pwd`/toolchain.cmake \
		..
        time make VERBOSE=1 -j12
        time make install

        popd
	fi
}


install() {
	echo ====grpc build process start====

	# check cmake have to greate and equal than v3.16.1
	check_cmake_version
	echo `pwd`
    GRPC_VERSION="v1.51.3"
	
	# clone
	if [ ! -d ${GRPC_VERSION} ];then
		time clone ${GRPC_VERSION}
	fi

    # build
	if [[ "${HOST}" = "Linux"  && "${TARGET}" == "linux64" ]];then
        build_grpc_linux64 ${GRPC_VERSION}
	elif [[ "${HOST}" = "Linux"  && "${TARGET}" == "win64" ]];then
		build_grpc_linux64 ${GRPC_VERSION}
        build_grpc_mingw_w64 ${GRPC_VERSION}
	elif [[ "${HOST}" = "Linux"  && "${TARGET}" == "aarch64" ]];then
		build_grpc_aarch64 ${GRPC_VERSION}
	fi
}

$@