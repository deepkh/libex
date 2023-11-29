#!/bin/bash

source check_cmake.sh
source grpc_helper.sh 

install() {
	echo ====grpc build process start====

	# check cmake version need greate and equal than v3.16.1
	check_cmake_version
	echo `pwd`
	
	# clone
	if [ ! -d "grpc" ];then
		time clone "v1.34.0" "grpc"
	fi

	# build host grpc to /usr/local
	if [ "`which grpc_cpp_plugin`" = "" ];then
		if [[ "${HOST}" = "Linux"  && "${TARGET}" == "linux64" ]];then
			build_host_grpc_and_install
		else
			echo "=== OOPs! grpc_cpp_plugin not found. please build Linux-x86_64 version first. ==="
			return 1
		fi
	fi
	
	# build target grpc to ${RUNTIME}
	if [ ! -f "${RUNTIME}/lib/libgrpc++.a" ];then
		build_target_grpc_and_install
	fi
}

$@
