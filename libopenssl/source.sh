#!/bin/bash

if [ ! -z "$1" ]; then
	unset LIBOPENSSL_PHONY
	unset LIBOPENSSL_PHONY_CLEAN
	if [ "${HAVE_LIB_OPENSSL}" = "1" ]; then
		export LIBOPENSSL_NAME="openssl-1.1.0f"
		export LIBOPENSSL_SUBNAME="tar.gz"
		export LIBOPENSSL_LIB="libssl.${LIBSUFFIX}"
		export LIBOPENSSL_CONFIG_H="Makefile"
		export LIBOPENSSL="$1"
		export LIBOPENSSL_PHONY="LIBOPENSSL"
		export LIBOPENSSL_PHONY_CLEAN="LIBOPENSSL_CLEAN"
		export LIBOPENSSL_CFLAGS=
		export LIBOPENSSL_LDFLAGS="-lssl -lcrypto"
		echo "LIBOPENSSL=${LIBOPENSSL}"
	fi
fi

