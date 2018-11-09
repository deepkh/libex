#!/bin/bash

if [ ! -z "$1" ]; then
	unset LIBFDKAAC_PHONY
	unset LIBFDKAAC_PHONY_CLEAN
	if [ "${HAVE_LIB_FDKAAC}" = "1" ]; then
		export LIBFDKAAC_NAME="fdk-aac-0.1.3"
		export LIBFDKAAC_SUBNAME="tar.gz"
		export LIBFDKAAC_LIB="libfdk-aac.${DLLASUFFIX}"
		export LIBFDKAAC_CONFIG_H="Makefile"
		export LIBFDKAAC="$1"
		export LIBFDKAAC_PHONY="LIBFDKAAC"
		export LIBFDKAAC_PHONY_CLEAN="LIBFDKAAC_CLEAN"
		export LIBFDKAAC_CFLAGS=
		export LIBFDKAAC_LDFLAGS="-lfdk-aac${LDLLSUFFIX}"
		echo "LIBFDKAAC=${LIBFDKAAC}"
	fi
fi

