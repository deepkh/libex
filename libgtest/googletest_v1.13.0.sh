#!/bin/bash

download_and_extract() {
	echo "==== $0 download ${1} and extraction ===="
	local TARCOMPRESS=${1}
	local FILENAME=${2}
	wget https://github.com/google/googletest/archive/refs/tags/${FILENAME}

	if [ -z "${FILENAME}" ];then
		echo "Failed to download ${FILENAME} !"
	else
		tar -${TARCOMPRESS}xvpf ${FILENAME}
	fi
}

build_target_and_install_linux64() {
	echo "==== $0 build_target_and_install_linux64 $1 ===="
	pushd ${1}

	rm -rf build
	mkdir build
	pushd build
	cmake -DCMAKE_INSTALL_PREFIX=${RUNTIME} ../
	make -j4
	make install
	popd
	
	popd
}

build_target_and_install_win64() {
	echo "==== $0 build_target_and_install_win64 $1 ===="
	pushd ${1}

	rm -rf build
	mkdir build
	pushd build

cat > toolchain.mingw-w64-x86_64.cmake <<'EOT'
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
set(CMAKE_CXX_STANDARD 11)
EOT
		  
		  #-DGTEST_HAS_PTHREAD=1

	cmake -DCMAKE_INSTALL_PREFIX=${RUNTIME} \
		  -DCMAKE_TOOLCHAIN_FILE=toolchain.mingw-w64-x86_64.cmake \
	..
	make -j16
	make install
	popd
	
	popd
}

build_target_and_install() {
	if [[ "${HOST}" = "Linux"  && "${TARGET}" == "linux64" ]];then
		build_target_and_install_linux64 ${1}
	elif [[ "${HOST}" = "Linux"  && "${TARGET}" == "win64" ]];then
		build_target_and_install_win64 ${1}
	fi
}

install() {
	echo ==== $0 install ====

	###
	### Failed to build v1.14.0 by use 'x86_64-w64-mingw32-g++ (GCC) 12-win32' due to
	### Googletest does not compile with POSIX Threads for Windows #3577
	### https://github.com/google/googletest/issues/3577
	### 
	### But to build v1.13.0 ok by use 'x86_64-w64-mingw32-g++ (GCC) 12-win32'
	### 

	#TARFILENAME=v1.14.0.tar.gz
	TARFILENAME=v1.13.0.tar.gz
	#EXTRACTED_FOLDER=googletest-1.14.0
	EXTRACTED_FOLDER=googletest-1.13.0
	BUILD_TARGET=${RUNTIME}/lib/libgtest.a

	# download and extract
	if [ ! -d ${EXTRACTED_FOLDER} ];then
		time download_and_extract z ${TARFILENAME}
	fi

	# build target and install to ${RUNTIME}
	if [ ! -f ${BUILD_TARGET} ];then
		build_target_and_install ${EXTRACTED_FOLDER}
	
		# if build process succeeded, create a _RESULT file
		if [ -f ${BUILD_TARGET} ];then
			echo ==== ${BUILD_TARGET} built success.
			touch ${LIBGTEST_RESULT}
		else
			echo ==== ${BUILD_TARGET} BUILD FAIL ==== 
		fi
	fi
}

$@

