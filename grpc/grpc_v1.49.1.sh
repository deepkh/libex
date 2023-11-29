#!/bin/bash

source check_cmake.sh

clone() {
	echo "=== clone grpc ${1} ; abseil-cpp ${2} ==="

	# checkout grpc
	git clone -b ${1} https://github.com/grpc/grpc ${3}
	pushd ${3}
	git submodule update --init	

	# checkouk absel-cpp to specific version
	pushd third_party/abseil-cpp
	git checkout -b ${2} tags/${2}
	popd

	popd 
}

build_abseil_cpp() {
	echo "=== build_abseil_cpp ==="
	mkdir -p grpc/third_party/abseil-cpp/build
	pushd grpc/third_party/abseil-cpp/build
	
	if [[ "${HOST}" = "Linux"  && "${TARGET}" == "linux64" ]];then
		cmake \
		-DCMAKE_INSTALL_PREFIX=${RUNTIME} \
		-DCMAKE_CXX_STANDARD=17 \
		-DCMAKE_POSITION_INDEPENDENT_CODE=TRUE \
		..
		time make -j16
		time make install
	elif [[ "${HOST}" = "Linux"  && "${TARGET}" == "win64" ]];then
		echo === DO_NOTHING. It will copy host_bin afterward. ===
	fi
	
	popd
}

build_grpc() {
	echo "=== build_grpc ==="
	mkdir -p grpc/build
	pushd grpc/build
	
	if [[ "${HOST}" = "Linux"  && "${TARGET}" == "linux64" ]];then
		cmake -DgRPC_INSTALL=ON \
				-DCMAKE_PREFIX_PATH="${RUNTIME}" \
				-DgRPC_ABSL_PROVIDER=package \
				-DgRPC_SSL_PROVIDER=package \
				-DgRPC_PROTOBUF_PROVIDER=package \
				-DCMAKE_BUILD_TYPE=Release \
				-DgRPC_BUILD_TESTS=OFF \
				-DCMAKE_CXX_STANDARD=17 \
				-DCMAKE_INSTALL_PREFIX=${RUNTIME} \
		..
		time make -j16
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
		#time clone "v1.59.3" "20230125.3" "grpc"
		time clone "v1.49.1" "20230125.3" "grpc"
		#time clone "v1.45.3" "20230125.3" "grpc"
	fi
	
	if [ ! -f "${RUNTIME}/lib/libabsl_time.a" ];then
		build_abseil_cpp
	fi

	# build host grpc to /usr/local
	if [ ! -f "${RUNTIME}/bin/grpc_cpp_plugin" ];then
		build_grpc
	fi
}

$@
