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
	unset LIBLODEPNG_PHONY
	unset LIBLODEPNG_PHONY_CLEAN
	if [ "${HAVE_LIB_LODEPNG}" = "1" ]; then
		export LIBLODEPNG_NAME="liblodepng"
		export LIBLODEPNG_LIB="${LIBLODEPNG_NAME}.${LIBSUFFIX}"
		export LIBLODEPNG="$1"
		export LIBLODEPNG_OBJS_DIR=${RUNTIME_OBJS}${LIBLODEPNG/${ROOT}/""}
		export LIBLODEPNG_PHONY="LIBLODEPNG"
		export LIBLODEPNG_PHONY_CLEAN="LIBLODEPNG_CLEAN"
		export LIBLODEPNG_CFLAGS=
		export LIBLODEPNG_LDFLAGS="-llodepng"
		echo "LIBLODEPNG=${LIBLODEPNG}"
		
		export LIBLODEPNG_HEADER_TRIGGER="lodepng.h"
	fi
fi

