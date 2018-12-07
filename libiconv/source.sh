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
	unset LIBICONV_PHONY
	unset LIBICONV_PHONY_CLEAN
	if [ "${HAVE_LIB_ICONV}" = "1" ]; then
		export LIBICONV_NAME="libiconv-1.14"
		export LIBICONV_SUBNAME="tar.gz"
		export LIBICONV_LIB="libiconv.${DLLASUFFIX}"
		export LIBICONV_CONFIG_H="config.h"
		export LIBICONV="$1"
		export LIBICONV_PHONY="LIBICONV"
		export LIBICONV_PHONY_CLEAN="LIBICONV_CLEAN"
		export LIBICONV_CFLAGS=
		export LIBICONV_LDFLAGS="-liconv${LDLLSUFFIX}"
		echo "LIBICONV=${LIBICONV}"
	fi
fi

