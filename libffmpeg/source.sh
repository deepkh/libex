# Copyright (c) 2018, Gary Huang, deepkh@gmail.com, https://github.com/deepkh
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#!/bin/bash

if [ ! -z "$1" ]; then
	unset LIBFFMPEG_PHONY
	unset LIBFFMPEG_PHONY_CLEAN
	if [ "${HAVE_LIB_FFMPEG}" = "1" ]; then
		if [ -z "LIBFFMPEG_VERSION" ]; then
			export LIBFFMPEG_VERSION="2.8.21"
		fi
		export LIBFFMPEG_NAME="ffmpeg-${LIBFFMPEG_VERSION}"
		export LIBFFMPEG_SUBNAME="tar.bz2"
		export LIBFFMPEG_CONFIG_H="was_configure"
		export LIBFFMPEG="$1"
		export LIBFFMPEG_OBJS_DIR=${RUNTIME_OBJS}${LIBFFMPEG/${ROOT}/""}
		export LIBFFMPEG_LIB="${RUNTIME_LIB}/libavcodec.${DLLASUFFIX}"
		export LIBFFMPEG_LIB_CLEAN="${LIBFFMPEG_LIB}_clean"
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

