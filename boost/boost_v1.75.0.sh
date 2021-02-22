#!/bin/bash

source boost_helper.sh 

install() {
	echo ====boost build process start====

	# download
	if [ ! -d "boost" ];then
		time download "1.75.0" "boost"
	fi

	# build target boost to ${RUNTIME}
	if [ ! -f "${RUNTIME}/lib/libboost_thread.a" ];then
		build_target_boost_and_install
	fi
}

$@

