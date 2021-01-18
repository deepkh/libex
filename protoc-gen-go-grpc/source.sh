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
	unset PROTOCGENGOGRPC_PHONY
	unset PROTOCGENGOGRPC_PHONY_CLEAN
	if [ "${HAVE_PROTOCGENGOGRPC}" = "1" ]; then
		export PROTOCGENGOGRPC_NAME="protoc-gen-go-grpc"
		export PROTOCGENGOGRPC_SUBNAME=""
		export PROTOCGENGOGRPC_CONFIG_H="was_configure"
		export PROTOCGENGOGRPC="$1"
		export PROTOCGENGOGRPC_OBJS_DIR=${RUNTIME_OBJS}${PROTOCGENGOGRPC/${ROOT}/""}
		export PROTOCGENGOGRPC_BIN="${GOBIN}/protoc-gen-go-grpc${HOST_BINSUFFIX}"
		export PROTOCGENGOGRPC_BIN_CLEAN="${PROTOCGENGOGRPC_BIN}_clean"
		export PROTOCGENGOGRPC_PHONY="PROTOCGENGOGRPC"
		export PROTOCGENGOGRPC_PHONY_CLEAN="PROTOCGENGOGRPC_CLEAN"
		export PROTOCGENGOGRPC_CFLAGS=
		export PROTOCGENGOGRPC_LDFLAGS=""
		echo "PROTOCGENGOGRPC=${PROTOCGENGOGRPC}"
	fi
fi

