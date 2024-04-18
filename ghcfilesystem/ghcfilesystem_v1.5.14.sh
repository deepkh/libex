#!/bin/bash

install() {
	echo ====grpc build process start====

	echo `pwd`
	
	# clone
	if [ ! -d "filesystem" ];then
		git clone -b v1.5.14 --depth 1 https://github.com/gulrak/filesystem
		cp -arpf filesystem/include/* $RUNTIME/include
	fi
}

$@
