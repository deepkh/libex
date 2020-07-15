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
	unset NSIS_PHONY
	unset NSIS_PHONY_CLEAN
	# Currently nsis only working on windows msys2
	if [ "${HAVE_NSIS}" = "1" ] && [ "${HOST}" = "MINGW32_NT" ]; then
		export NSIS_NAME="nsis-3.01"
		export NSIS_SUBNAME="zip"
		export NSIS_CONFIG_H="was_configure"
		export NSIS="$1"
		export NSIS_OBJS_DIR=${RUNTIME_OBJS}${NSIS/${ROOT}/""}
		export NSIS_BIN="${NSIS_OBJS_DIR}/${NSIS_NAME}/makensis${HOST_BINSUFFIX}"
		export NSIS_BIN_CLEAN="${NSIS_BIN}_clean"
		export NSIS_PHONY="NSIS"
		export NSIS_PHONY_CLEAN="NSIS_CLEAN"
		export NSIS_CFLAGS=
		export NSIS_LDFLAGS=""
		echo "NSIS=${NSIS}"

		#export env
		export NSISROOT=${NSIS_OBJS_DIR}/${NSIS_NAME}
		export MAKENSISBIN=${NSIS_OBJS_DIR}/${NSIS_NAME}/nsis${HOST_BINSUFFIX}
		export MAKENSISBIN=${NSIS_OBJS_DIR}/${NSIS_NAME}/makensis${HOST_BINSUFFIX}
	fi
fi

