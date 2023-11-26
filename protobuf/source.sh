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
	unset PROTOBUF_PHONY
	unset PROTOBUF_PHONY_CLEAN
	if [ "${HAVE_PROTOBUF}" = "1" ]; then
		# Build Host Binary 
		export PROTOBUF_DOWNLOAD_NAME="v3.20.2"
		export PROTOBUF_NAME="protobuf-3.20.2"
		export PROTOBUF_SUBNAME="tar.gz"
		export PROTOBUF_CONFIG_H="was_configure"
		export PROTOBUF="$1"
		export PROTOBUF_OBJS_DIR=${RUNTIME_OBJS}${PROTOBUF/${ROOT}/""}
		export PROTOBUF_BIN="${PROTOBUF_OBJS_DIR}/${PROTOBUF_NAME}-bin/bin/protoc${HOST_BINSUFFIX}"
		export PROTOBUF_BIN_CLEAN="${PROTOBUF_BIN}_clean"
		export PROTOBUF_PHONY="PROTOBUF"
		export PROTOBUF_PHONY_CLEAN="PROTOBUF_CLEAN"
		export PROTOBUF_CFLAGS=
		export PROTOBUF_LDFLAGS=""
		export PROTOC=${PROTOBUF_BIN}
		echo "PROTOBUF=${PROTOBUF}"

	fi
fi

