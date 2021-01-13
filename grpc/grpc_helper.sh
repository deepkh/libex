#!/bin/bash

clone() {
	echo "=== clone grpc ${1} ==="
	git clone -b ${1} --depth 1 --recursive https://github.com/grpc/grpc.git ${2}
}

build_host_grpc_and_install() {
	echo "=== build_host_grpc_and_install ==="
	mkdir -p grpc/cmake/build_host
	pushd grpc/cmake/build_host

	#rm -rf *

	cmake -DgRPC_INSTALL=ON \
			-DCMAKE_BUILD_TYPE=Release \
			-DgRPC_BUILD_TESTS=OFF \
			-DCMAKE_INSTALL_PREFIX=/usr/local \
      ../..
	time make -j8
	time sudo make install
	popd
}

build_target_grpc_and_install_linux64() {
	echo "=== build_target_grpc_and_install_linux64 ==="
	mkdir -p grpc/cmake/build_linux64
	pushd grpc/cmake/build_linux64

	#rm -rf *

	cmake -DgRPC_INSTALL=ON \
			-DCMAKE_BUILD_TYPE=Release \
			-DgRPC_BUILD_TESTS=OFF \
			-DCMAKE_INSTALL_PREFIX=${RUNTIME} \
      ../..
	time make -j8
	time make install
	popd
}

build_target_grpc_and_install_win64() {
	echo "=== build_target_grpc_and_install_win64 ==="
	mkdir -p grpc/cmake/build_win64
	pushd grpc/cmake/build_win64

	#rm -rf *

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
set(CMAKE_CXX_STANDARD 11)
EOT

cmake -DgRPC_INSTALL=ON \
		-DCMAKE_BUILD_TYPE=Release \
		-DgRPC_BUILD_TESTS=OFF \
		-DCMAKE_INSTALL_PREFIX=${RUNTIME} \
		-DCMAKE_TOOLCHAIN_FILE=`pwd`/toolchain.cmake \
		../..

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

