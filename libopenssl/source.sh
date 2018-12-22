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
	unset LIBOPENSSL_PHONY
	unset LIBOPENSSL_PHONY_CLEAN
	if [ "${HAVE_LIB_OPENSSL}" = "1" ]; then
		export LIBOPENSSL_NAME="openssl-1.1.0f"
		export LIBOPENSSL_SUBNAME="tar.gz"
		export LIBOPENSSL_LIB="libssl.${LIBSUFFIX}"
		export LIBOPENSSL_CONFIG_H="is_configured"
		export LIBOPENSSL="$1"
		export LIBOPENSSL_OBJS_DIR=${RUNTIME_OBJS}${LIBOPENSSL/${ROOT}/""}
		export LIBOPENSSL_PHONY="LIBOPENSSL"
		export LIBOPENSSL_PHONY_CLEAN="LIBOPENSSL_CLEAN"
		export LIBOPENSSL_CFLAGS=
		export LIBOPENSSL_LDFLAGS="-lssl -lcrypto"
		echo "LIBOPENSSL=${LIBOPENSSL}"
	fi
fi

