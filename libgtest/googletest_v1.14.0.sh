#!/bin/bash

source googletest_helper.sh 

install() {
	echo ==== $0 install ====

	BUILD_VERSION=1.14.0
	BUILD_FOLDER=googletest
	BUILD_TARGET=${RUNTIME}/lib/libgtest.a

	# download
	if [ ! -d ${BUILD_FOLDER} ];then
		time download ${BUILD_VERSION} ${BUILD_FOLDER}
	fi

	# build target and install to ${RUNTIME}
	if [ ! -f ${BUILD_TARGET} ];then
		build_target_and_install ${BUILD_FOLDER}
	
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

