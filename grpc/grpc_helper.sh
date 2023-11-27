#!/bin/bash

clone() {
	echo "=== clone grpc ${1} ==="
	git clone -b ${1} https://github.com/grpc/grpc ${2}
	pushd grpc
	git submodule update --init	
	popd 
}

build_host_grpc_and_install() {
	echo "=== build_host_grpc_and_install xxxx1 ==="
	mkdir -p grpc/build_host
	pushd grpc/build_host
	
	if [[ "${HOST}" = "Linux"  && "${TARGET}" == "linux64" ]];then
		cmake -DgRPC_INSTALL=ON \
				-DCMAKE_PREFIX_PATH=${RUNTIME} \
				-DgRPC_SSL_PROVIDER=package \
				-DgRPC_PROTOBUF_PROVIDER=package \
				-DCMAKE_BUILD_TYPE=Release \
				-DgRPC_BUILD_TESTS=OFF \
				-DCMAKE_INSTALL_PREFIX=${GRPC_HOST_BIN_DIR} \
		..
		time make -j8
		time make install
	elif [[ "${HOST}" = "Linux"  && "${TARGET}" == "win64" ]];then
		#
		# [WORKAROUD]
		# copy previous built of $ROOT/runtime.linux64 to $GRPC_HOST_BIN_DIR
		# It does mean it has to build a Linux 64-bit version first.
		# 
		if [ ! -d "${ROOT}/runtime.linux64/objs/libex/grpc/host_bin/bin/" ];then
			echo "====== ${ROOT}/runtime.linux64/objs/libex/grpc/host_bin NOT FOUND! Please build linux64 version first!!!!!"
		else
			echo copy ${ROOT}/runtime.linux64/objs/libex/grpc/host_bin to ${GRPC_HOST_BIN_DIR}
			cp -arpf ${ROOT}/runtime.linux64/objs/libex/grpc/host_bin ${GRPC_HOST_BIN_DIR}
		fi
	fi
	
	popd
}

build_target_grpc_and_install_linux64() {
	echo "=== build_target_grpc_and_install_linux64 ==="
	cp -arpf ${GRPC_HOST_BIN_DIR}/bin/* ${RUNTIME}/bin
	cp -arpf ${GRPC_HOST_BIN_DIR}/lib/* ${RUNTIME}/lib
	cp -arpf ${GRPC_HOST_BIN_DIR}/include/* ${RUNTIME}/include
}

#
# 20231127
# It does not work anymore to cross-compile from Debian since grpc_v1.49.1
#
build_target_grpc_and_install_win64() {
	echo "=== build_target_grpc_and_install_win64 ==="
	mkdir -p grpc/build_win64
	pushd grpc/build_win64

	#rm -rf *

cat > toolchain.cmake <<'EOT'
SET(CMAKE_SYSTEM_NAME Windows)
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
SET(CMAKE_SYSTEM_PROCESSOR x86_64)
set(CMAKE_SYSROOT /usr/x86_64-w64-mingw32)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc-posix)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++-posix)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)
set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX} /usr/lib/gcc/${TOOLCHAIN_PREFIX}/12-posix)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
set(CMAKE_CXX_STANDARD 17)
EOT

cmake -DgRPC_INSTALL=ON \
		-DCMAKE_BUILD_TYPE=Release \
		-DgRPC_BUILD_TESTS=OFF \
		-DCMAKE_INSTALL_PREFIX=${RUNTIME} \
		-DCMAKE_TOOLCHAIN_FILE=`pwd`/toolchain.cmake \
		..

	time make -j8
	time make install
	popd
}

build_target_grpc_and_install() {
	if [[ "${HOST}" = "Linux"  && "${TARGET}" == "linux64" ]];then
		build_target_grpc_and_install_linux64
	elif [[ "${HOST}" = "Linux"  && "${TARGET}" == "win64" ]];then
		build_target_grpc_and_install_win64
	fi
}

