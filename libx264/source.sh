#!/bin/bash

if [ ! -z "$1" ]; then
	unset LIBX264_PHONY
	unset LIBX264_PHONY_CLEAN
	if [ "${HAVE_LIB_X264}" = "1" ]; then
		export LIBX264_NAME="x264-snapshot-20140712-2245-stable"
		export LIBX264_SUBNAME="tar.bz2"
		export LIBX264_LIB="libx264.${DLLASUFFIX}"
		export LIBX264_CONFIG_H="config.h"
		export LIBX264="$1"
		export LIBX264_PHONY="LIBX264"
		export LIBX264_PHONY_CLEAN="LIBX264_CLEAN"
		export LIBX264_CFLAGS=
		export LIBX264_LDFLAGS="-lx264${LDLLSUFFIX}"
		echo "LIBX264=${LIBX264}"
	fi
fi

