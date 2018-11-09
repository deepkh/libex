#!/bin/bash

if [ ! -z "$1" ]; then
	unset LIBLODEPNG_PHONY
	unset LIBLODEPNG_PHONY_CLEAN
	if [ "${HAVE_LIB_LODEPNG}" = "1" ]; then
		export LIBLODEPNG_NAME="liblodepng"
		export LIBLODEPNG_LIB="${LIBLODEPNG_NAME}.${LIBSUFFIX}"
		export LIBLODEPNG="$1"
		export LIBLODEPNG_PHONY="LIBLODEPNG"
		export LIBLODEPNG_PHONY_CLEAN="LIBLODEPNG_CLEAN"
		export LIBLODEPNG_CFLAGS=
		export LIBLODEPNG_LDFLAGS="-llodepng"
		echo "LIBLODEPNG=${LIBLODEPNG}"
		
		export LIBLODEPNG_HEADER_TRIGGER="lodepng.h"
	fi
fi

