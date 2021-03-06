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
	unset I686W64MINGW32DLLS_PHONY
	unset I686W64MINGW32DLLS_PHONY_CLEAN
	if [ "${HAVE_I686W64MINGW32DLLS}" = "1" ]; then
		export I686W64MINGW32DLLS_NAME="i686-w64-mingw32-4.9.1-poix-dlls-jessie"
		export I686W64MINGW32DLLS_SUBNAME="tar.xz"
		export I686W64MINGW32DLLS_CONFIG_H="was_configure"
		export I686W64MINGW32DLLS="$1"
		export I686W64MINGW32DLLS_OBJS_DIR=${RUNTIME_OBJS}${I686W64MINGW32DLLS/${ROOT}/""}
		export I686W64MINGW32DLLS_BIN="${RUNTIME_BIN}/libgcc_s_sjlj-1.dll"
		export I686W64MINGW32DLLS_BIN_CLEAN="${I686W64MINGW32DLLS_BIN}_clean"
		export I686W64MINGW32DLLS_PHONY="I686W64MINGW32DLLS"
		export I686W64MINGW32DLLS_PHONY_CLEAN="I686W64MINGW32DLLS_CLEAN"
		export I686W64MINGW32DLLS_CFLAGS=
		export I686W64MINGW32DLLS_LDFLAGS=""
		echo "I686W64MINGW32DLLS=${I686W64MINGW32DLLS}"
	fi
fi

