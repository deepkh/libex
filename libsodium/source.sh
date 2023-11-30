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
	unset LIBSODIUM_PHONY
	unset LIBSODIUM_PHONY_CLEAN
	if [ "${HAVE_LIB_LIBSODIUM}" = "1" ]; then
		export LIBSODIUM_VERSION="1.0.19"
		export LIBSODIUM_NAME="libsodium-${LIBSODIUM_VERSION}"
		export LIBSODIUM_SUBNAME="tar.gz"
		export LIBSODIUM_CONFIG_H="is_configured"
		export LIBSODIUM="$1"
		export LIBSODIUM_OBJS_DIR=${RUNTIME_OBJS}${LIBSODIUM/${ROOT}/""}
		export LIBSODIUM_LIB="${RUNTIME_LIB}/libsodium.${DLLASUFFIX}"
		export LIBSODIUM_LIB_CLEAN="${LIBSODIUM_LIB}_clean"
		export LIBSODIUM_PHONY="LIBSODIUM"
		export LIBSODIUM_PHONY_CLEAN="LIBSODIUM_CLEAN"
		export LIBSODIUM_CFLAGS=
		export LIBSODIUM_LDFLAGS="-lsodium${LDLLSUFFIX}"
		echo "LIBSODIUM=${LIBSODIUM}"
	fi
fi

