#!/bin/bash

if [ ! -z "$1" ]; then
	unset LIBICONV_PHONY
	unset LIBICONV_PHONY_CLEAN
	if [ "${HAVE_LIB_ICONV}" = "1" ]; then
		export LIBICONV_NAME="libiconv-1.14"
		export LIBICONV_SUBNAME="tar.gz"
		export LIBICONV_LIB="libiconv.${DLLASUFFIX}"
		export LIBICONV_CONFIG_H="config.h"
		export LIBICONV="$1"
		export LIBICONV_PHONY="LIBICONV"
		export LIBICONV_PHONY_CLEAN="LIBICONV_CLEAN"
		export LIBICONV_CFLAGS=
		export LIBICONV_LDFLAGS="-liconv${LDLLSUFFIX}"
		echo "LIBICONV=${LIBICONV}"
	fi
fi

