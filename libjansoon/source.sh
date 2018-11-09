#!/bin/bash

if [ ! -z "$1" ]; then
	unset LIBJANSSON_PHONY
	unset LIBJANSSON_PHONY_CLEAN
	if [ "${HAVE_LIB_JANSSON}" = "1" ]; then
		export LIBJANSSON_NAME="jansson-2.7"
		export LIBJANSSON_SUBNAME="tar.gz"
		export LIBJANSSON_LIB="libjansson.${LIBSUFFIX}"
		export LIBJANSSON_CONFIG_H="Makefile"
		export LIBJANSSON="$1"
		export LIBJANSSON_PHONY="LIBJANSSON"
		export LIBJANSSON_PHONY_CLEAN="LIBJANSSON_CLEAN"
		export LIBJANSSON_CFLAGS=
		export LIBJANSSON_LDFLAGS="-ljansson"
		echo "LIBJANSSON=${LIBJANSSON}"
	fi
fi

