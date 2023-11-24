#!/bin/bash

download() {
	echo "==== $0 download googletest-v${1} and extract to `pwd`/${2} ===="
	local VERSION=${1}
	local BUILD_FOLDER=${2}
	local FILENAME=v${VERSION}.tar.gz
	wget https://github.com/google/googletest/archive/refs/tags/${FILENAME}
	tar -zxvpf ${FILENAME}
	ls googletest-${VERSION}
	ln -sf googletest-${VERSION} ${BUILD_FOLDER}
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

build_target_and_install() {
	if [[ "${HOST}" = "Linux"  && "${TARGET}" == "linux64" ]];then
		build_target_and_install_linux64 ${1}
	elif [[ "${HOST}" = "Linux"  && "${TARGET}" == "win64" ]];then
		build_target_and_install_win64 ${1}
	fi
}

