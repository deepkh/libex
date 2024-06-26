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
	unset GRPC_PHONY
	unset GRPC_PHONY_CLEAN
	if [ "${HAVE_GRPC}" = "1" ]; then
		export GRPC_NAME="grpc_v1.51.3"
		export GRPC_SUBNAME="sh"
		export GRPC="$1"
		export GRPC_OBJS_DIR=${RUNTIME_OBJS}${GRPC/${ROOT}/""}
		export GRPC_HOST_BIN_DIR=${GRPC_OBJS_DIR}/host_bin
		export GRPC_RESULT="${GRPC_OBJS_DIR}/${GRPC_NAME}.${GRPC_SUBNAME}_RESULT"
		export GRPC_RESULT_CLEAN="${GRPC_RESULT}_clean"
		export GRPC_PHONY="GRPC"
		export GRPC_PHONY_CLEAN="GRPC_CLEAN"
		export GRPC_CFLAGS=
		export GRPC_LDFLAGS="`PKG_CONFIG_PATH=${RUNTIME}/grpc-v1.51.3/lib/pkgconfig pkg-config --libs protobuf grpc++` -lpthread -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-neede"
    	export LIBPROTOBUF_LDFLAGS="-lprotobuf -lpthread"
		echo "GRPC=${GRPC}"

        # workaround for mingw-w64
	    if  [[ "${HOST}" = "Linux"  && "${TARGET}" == "win64" ]];then
		    export PATH=${ROOT}/runtime.linux64/grpc-v1.51.3/bin/:$PATH
        fi
	fi
fi