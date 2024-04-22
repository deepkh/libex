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
	unset LIBFDKAAC_PHONY
	unset LIBFDKAAC_PHONY_CLEAN
	if [ "${HAVE_LIB_FDKAAC}" = "1" ]; then
		export LIBFDKAAC_NAME="fdk-aac-2.0.3"
		export LIBFDKAAC_SUBNAME="tar.gz"
		export LIBFDKAAC_CONFIG_H="was_configure"
		export LIBFDKAAC="$1"
		export LIBFDKAAC_OBJS_DIR=${RUNTIME_OBJS}${LIBFDKAAC/${ROOT}/""}
		export LIBFDKAAC_LIB="${RUNTIME_LIB}/libfdk-aac.${DLLASUFFIX}"
		export LIBFDKAAC_LIB_CLEAN="${LIBFDKAAC_LIB}_clean"
		export LIBFDKAAC_PHONY="LIBFDKAAC"
		export LIBFDKAAC_PHONY_CLEAN="LIBFDKAAC_CLEAN"
		export LIBFDKAAC_CFLAGS=
		export LIBFDKAAC_LDFLAGS="-lfdk-aac${LDLLSUFFIX}"
		echo "LIBFDKAAC=${LIBFDKAAC}"
	fi
fi

