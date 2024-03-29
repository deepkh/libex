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
	unset LIBGTEST_PHONY
	unset LIBGTEST_PHONY_CLEAN
	if [ "${HAVE_LIBGTEST}" = "1" ]; then
		export LIBGTEST_NAME="googletest_v1.13.0"
		export LIBGTEST_SUBNAME="sh"
		export LIBGTEST="$1"
		export LIBGTEST_OBJS_DIR=${RUNTIME_OBJS}${LIBGTEST/${ROOT}/""}
		export LIBGTEST_RESULT="${LIBGTEST_OBJS_DIR}/${LIBGTEST_NAME}.${LIBGTEST_SUBNAME}_RESULT"
		export LIBGTEST_RESULT_CLEAN="${LIBGTEST_RESULT}_clean"
		export LIBGTEST_PHONY="LIBGTEST"
		export LIBGTEST_PHONY_CLEAN="LIBGTEST_CLEAN"
		export LIBGTEST_CFLAGS=
		export LIBGTEST_LDFLAGS=""
		echo "LIBGTEST=${LIBGTEST}"
	fi
fi

