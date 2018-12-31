# Copyright (c) 2018, Gary Huang, deepkh@gmail.com, https://github.com/deepkh
#
# Permission to use, copy, modify, and distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO PROTOBUF SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#!/bin/bash

if [ ! -z "$1" ]; then
	unset LIBPROTOBUF_PHONY
	unset LIBPROTOBUF_PHONY_CLEAN
	if [ "${HAVE_LIB_PROTOBUF}" = "1" ]; then
		export LIBPROTOBUF_DOWNLOAD_NAME="v3.6.1"
		export LIBPROTOBUF_NAME="protobuf-3.6.1"
		export LIBPROTOBUF_SUBNAME="tar.gz"
		export LIBPROTOBUF_CONFIG_H="was_configure"
		export LIBPROTOBUF="$1"
		export LIBPROTOBUF_OBJS_DIR=${RUNTIME_OBJS}${LIBPROTOBUF/${ROOT}/""}
		export LIBPROTOBUF_LIB="${RUNTIME_LIB}/libprotobuf.${LIBSUFFIX}"
		export LIBPROTOBUF_LIB_CLEAN="${LIBPROTOBUF_LIB}_clean"
		export LIBPROTOBUF_PHONY="LIBPROTOBUF"
		export LIBPROTOBUF_PHONY_CLEAN="LIBPROTOBUF_CLEAN"
		export LIBPROTOBUF_CFLAGS=
		# msys2 mingw only produced libprotobuf.a not included the libprotobuf.dll.a
		# but we desired .dll first
		if [[ "${HOST}" = "MINGW32_NT"  && "${TARGET}" = "win32" ]];then
			export LIBPROTOBUF_LDFLAGS="-lprotobuf"
		else
			export LIBPROTOBUF_LDFLAGS="-lprotobuf${LDLLSUFFIX}"
		fi
		echo "LIBPROTOBUF=${LIBPROTOBUF}"
	fi
fi

