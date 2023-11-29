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
	unset LIBZMQ_PHONY
	unset LIBZMQ_PHONY_CLEAN
	if [ "${HAVE_LIB_LIBZMQ}" = "1" ]; then
		export LIBZMQ_VERSION="4.3.5"
		export LIBZMQ_NAME="zeromq-${LIBZMQ_VERSION}"
		export LIBZMQ_SUBNAME="tar.gz"
		export LIBZMQ_CONFIG_H="is_configured"
		export LIBZMQ="$1"
		export LIBZMQ_OBJS_DIR=${RUNTIME_OBJS}${LIBZMQ/${ROOT}/""}
		export LIBZMQ_LIB="${RUNTIME_LIB}/libzmq.${DLLASUFFIX}"
		export LIBZMQ_LIB_CLEAN="${LIBZMQ_LIB}_clean"
		export LIBZMQ_PHONY="LIBZMQ"
		export LIBZMQ_PHONY_CLEAN="LIBZMQ_CLEAN"
		export LIBZMQ_CFLAGS=
		export LIBZMQ_LDFLAGS="-lzmq${LDLLSUFFIX}"
		echo "LIBZMQ=${LIBZMQ}"
	fi
fi

