#!/bin/bash

if [ ! -z "$1" ]; then
	unset LIBFFMPEG_PHONY
	unset LIBFFMPEG_PHONY_CLEAN
	if [ "${HAVE_LIB_FFMPEG}" = "1" ]; then
		export LIBFFMPEG_NAME="ffmpeg-2.8.7"
		export LIBFFMPEG_SUBNAME="tar.bz2"
		export LIBFFMPEG_LIB="libavcodec.${DLLASUFFIX}"
		export LIBFFMPEG_CONFIG_H="Makefile"
		export LIBFFMPEG="$1"
		export LIBFFMPEG_PHONY="LIBFFMPEG"
		export LIBFFMPEG_PHONY_CLEAN="LIBFFMPEG_CLEAN"
		export LIBFFMPEG_CFLAGS=
		export LIBFFMPEG_LDFLAGS="-lavcodec${LDLLSUFFIX} \
								-lavfilter${LDLLSUFFIX} \
								-lavformat${LDLLSUFFIX} \
								-lavresample${LDLLSUFFIX} \
								-lavutil${LDLLSUFFIX} \
								-lswresample${LDLLSUFFIX} \
								-lswscale${LDLLSUFFIX}
								#-lpostproc${LDLLSUFFIX}"
		echo "LIBFFMPEG=${LIBFFMPEG}"
	fi
fi

