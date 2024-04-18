#!/bin/bash

download() {
	echo "==== $0 download boost_${1} and extract to `pwd`/${2} ===="

	local VER=${1//./_}
	local FILENAME=boost_${VER}.tar.bz2
	wget https://boostorg.jfrog.io/artifactory/main/release/${1}/source/${FILENAME}
	tar -jxvpf ${FILENAME}
	ls ${FILENAME/.tar.bz2//}
	ln -sf ${FILENAME/.tar.bz2//} ${2}
}

build_host_boost_and_install() {
	echo "=== build_host_boost_and_install ==="
	mkdir -p boost/cmake/build_host
	pushd boost/cmake/build_host

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

build_target_and_install_linux64() {
	echo "==== $0 build_target_and_install_linux64 $1 ===="

	pushd boost
	
	local BOOST_BUILD_DIR=/tmp/boost_build_dir
	rm -rf stage ${RUNTIME}/lib/libboost* ${RUNTIME}/include/boost ${RUNTIME}/lib/cmake/boost_* ${BOOST_BUILD_DIR} ~/user-config.jam
	CC= CXX= ./bootstrap.sh --prefix=${RUNTIME} --without-libraries=python
	./b2 --clean-all 
	time ./b2 --build-dir=${BOOST_BUILD_DIR} --prefix=${RUNTIME} \
		variant=release \
		link=static runtime-link=static address-model=64 --without-python install

	popd
}

build_target_and_install_linux_aarch64() {
	echo "==== $0 build_target_and_install_linux_aarch64 $1 ===="

	pushd boost
	
	local BOOST_BUILD_DIR=/tmp/boost_build_dir
	rm -rf stage ${RUNTIME}/lib/libboost* ${RUNTIME}/include/boost ${RUNTIME}/lib/cmake/boost_* ${BOOST_BUILD_DIR} ~/user-config.jam
	CC= CXX= ./bootstrap.sh --prefix=${RUNTIME} --without-libraries=python
	./b2 --clean-all 
	time ./b2 --build-dir=${BOOST_BUILD_DIR} --prefix=${RUNTIME} \
		variant=release \
		link=static runtime-link=static address-model=64 --without-python install

	popd
}


build_target_and_install_win64() {
	echo "==== $0 build_target_and_install_linux64 $1 ===="

	pushd boost
	
	local BOOST_MINGW_BUILD_DIR=/tmp/boost_mingw_w64_build_dir
	rm -rf stage ${RUNTIME}/lib/libboost* ${RUNTIME}/include/boost ${RUNTIME}/lib/cmake/boost_* ${BOOST_MINGW_BUILD_DIR} ~/user-config.jam
	CC= CXX= ./bootstrap.sh --prefix=${RUNTIME} --without-libraries=python
	./b2 --clean-all 
	echo "using gcc : mingw : x86_64-w64-mingw32-g++ ;" > ~/user-config.jam
	time ./b2 --build-dir=${BOOST_MINGW_BUILD_DIR} --prefix=${RUNTIME} \
		toolset=gcc-mingw \
		target-os=windows \
		variant=release \
		link=static runtime-link=static address-model=64 --without-python install

	popd
}

build_target_and_install() {
	if [[ "${HOST}" = "Linux"  && "${TARGET}" == "linux64" ]];then
		build_target_and_install_linux64
	elif [[ "${HOST}" = "Linux"  && "${TARGET}" == "aarch64" ]];then
		build_target_and_install_linux_aarch64
	elif [[ "${HOST}" = "Linux"  && "${TARGET}" == "win64" ]];then
		build_target_and_install_win64
	fi
}

