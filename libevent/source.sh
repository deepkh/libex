#!/bin/bash

if [ ! -z "$1" ]; then
	unset LIBEVENT_PHONY
	unset LIBEVENT_PHONY_CLEAN
	if [ "${HAVE_LIB_EVENT}" = "1" ]; then
		export LIBEVENT_NAME="libevent-2.1.8-stable"
		export LIBEVENT_SUBNAME="tar.gz"
		export LIBEVENT_LIB="libevent.${LIBSUFFIX}"
		export LIBEVENT_CONFIG_H="Makefile"
		export LIBEVENT="$1"
		export LIBEVENT_PHONY="LIBEVENT"
		export LIBEVENT_PHONY_CLEAN="LIBEVENT_CLEAN"
		export LIBEVENT_CFLAGS=
		export LIBEVENT_LDFLAGS="-levent -levent_core -levent_extra -levent_openssl"
		echo "LIBEVENT=${LIBEVENT}"
	fi
fi

