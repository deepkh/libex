#!/bin/bash

if [ ! -z "$1" ]; then
	unset LIBKLIB_PHONY
	unset LIBKLIB_PHONY_CLEAN
	if [ "${HAVE_LIB_KLIB}" = "1" ]; then
		export LIBKLIB_NAME="klib"
		export LIBKLIB_HEADER_TRIGGER="khash.h"
		export LIBKLIB="$1"
		export LIBKLIB_PHONY="LIBKLIB"
		export LIBKLIB_PHONY_CLEAN="LIBKLIB_CLEAN"
		export LIBKLIB_CFLAGS=
		export LIBKLIB_LDFLAGS=
		echo "LIBKLIB=${LIBKLIB}"
		
		export LIBKLIB_HEADER_FAKE_TRIGGER="khash_fake.h"
	fi
fi


