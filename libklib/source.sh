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
	unset LIBKLIB_PHONY
	unset LIBKLIB_PHONY_CLEAN
	if [ "${HAVE_LIB_KLIB}" = "1" ]; then
		export LIBKLIB_NAME="klib"
		export LIBKLIB="$1"
		export LIBKLIB_HEAD_HEADER="${RUNTIME_INCLUDE}/${LIBKLIB_NAME}/khash.h"
		export LIBKLIB_HEAD_HEADER_CLEAN="${LIBKLIB_HEAD_HEADER}_clean"
		export LIBKLIB_PHONY="LIBKLIB"
		export LIBKLIB_PHONY_CLEAN="LIBKLIB_CLEAN"
		export LIBKLIB_CFLAGS=
		export LIBKLIB_LDFLAGS=
		echo "LIBKLIB=${LIBKLIB}"
	fi
fi


