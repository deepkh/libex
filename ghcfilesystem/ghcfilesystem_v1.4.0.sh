#!/bin/bash

install() {
	echo ====grpc build process start====

	echo `pwd`
	
	# clone
	if [ ! -d "filesystem" ];then
		git clone -b v1.4.0 --depth 1 https://github.com/gulrak/filesystem
	fi
}

$@
