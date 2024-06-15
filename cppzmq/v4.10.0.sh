#!/bin/bash

clone() {
	echo "=== clone zeromq-${1} ;  ==="

	wget "https://github.com/zeromq/cppzmq/archive/refs/tags/${1}.tar.gz"

	tar -zxvpf ${1}.tar.gz
	mv cppzmq-4.10.0 ${1}
}

build_cppzmq_linux64() {

	if [ ! -d ${1}/build_linux64 ];then
    	echo "=== build_cppzmq_linux64 ${1} ==="

    	mkdir -p ${1}/build_linux64
    	pushd ${1}/build_linux64

        cmake -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_CXX_STANDARD=17 \
            -DgRPC_BUILD_TESTS=OFF \
            -DCMAKE_INSTALL_PREFIX=${RUNTIME} \
        ..
        time make VERBOSE=1 -j8
        time make install

        popd
	fi
}

build_cppzmq_aarch64() {

	if [ ! -d ${1}/build_aarch64 ];then
    	echo "=== build_cppzmq_linux_aarch64 ${1} ==="

    	mkdir -p ${1}/build_aarch64
    	pushd ${1}/build_aarch64

        cmake -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_CXX_STANDARD=17 \
            -DgRPC_BUILD_TESTS=OFF \
            -DCMAKE_INSTALL_PREFIX=${RUNTIME} \
        ..
        time make VERBOSE=1 -j8
        time make install

        popd
	fi
}


build_cppzmq_mingw_w64() {

	if [ ! -d ${1}/build_mingw-w64 ];then
    	echo "=== build_cppzmq_mingw-w64 ${1} ==="

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
set(CMAKE_CXX_FLAGS "-fpermissive")
EOT

        cmake -DCMAKE_BUILD_TYPE=Release \
		-DgRPC_BUILD_TESTS=OFF \
		-DCMAKE_INSTALL_PREFIX=${RUNTIME} \
		-DCMAKE_TOOLCHAIN_FILE=`pwd`/toolchain.cmake \
		-DZeroMQ_INCLUDE_DIR=${RUNTIME}/include \
		-DZeroMQ_LIBRARY=${RUNTIME}/lib/libzmq.dll.a \
		-DZeroMQ_VERSION=4.3.5 \
		..
        time make VERBOSE=1 -j8
        time make VERBOSE=1 install

        popd
	fi
}


install() {
	echo ====CPPZMQ build process start====

	# check cmake have to greate and equal than v3.16.1
	echo `pwd`
    CPPZMQ_VERSION="v4.10.0"
	
	# clone
	if [ ! -d ${CPPZMQ_VERSION} ];then
		time clone ${CPPZMQ_VERSION}
	fi

    # build
	if [[ "${HOST}" = "Linux"  && "${TARGET}" == "linux64" ]];then
        build_cppzmq_linux64 ${CPPZMQ_VERSION}
	elif [[ "${HOST}" = "Linux"  && "${TARGET}" == "win64" ]];then
        build_cppzmq_mingw_w64 ${CPPZMQ_VERSION}
	elif [[ "${HOST}" = "Linux"  && "${TARGET}" == "aarch64" ]];then
		build_cppzmq_aarch64 ${CPPZMQ_VERSION}
	fi
}

$@