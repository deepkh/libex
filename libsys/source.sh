#!/bin/bash

if [ ! -z "$1" ]; then
	unset LIBSYS_PHONY
	unset LIBSYS_PHONY_CLEAN
	if [ "${HAVE_LIB_SYS}" = "1" ]; then
		export LIBSYS_NAME="sys"
		export LIBSYS_HEADER_TRIGGER="queue.h"
		export LIBSYS="$1"
		export LIBSYS_PHONY="LIBSYS"
		export LIBSYS_PHONY_CLEAN="LIBSYS_CLEAN"
		export LIBSYS_CFLAGS=
		export LIBSYS_LDFLAGS=
		echo "LIBSYS=${LIBSYS}"
	fi
fi


