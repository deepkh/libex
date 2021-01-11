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
	unset GOCOMPILER_PHONY
	unset GOCOMPILER_PHONY_CLEAN
	if [ "${HAVE_GOCOMPILER}" = "1" ]; then
		if [ "${HOST}" = "Linux" ];then
			export GOCOMPILER_NAME="go1.15.6.linux-amd64"
			export GOCOMPILER_SUBNAME="tar.gz"
		elif [ "${HOST}" = "MINGW32_NT" ];then
			export GOCOMPILER_NAME="go1.15.6.windows-amd64"
			export GOCOMPILER_SUBNAME="zip"
		fi
		export GOCOMPILER_CONFIG_H="was_configure"
		export GOCOMPILER="$1"
		export GOCOMPILER_OBJS_DIR=${RUNTIME_OBJS}${GOCOMPILER/${ROOT}/""}
		export GOCOMPILER_BIN="${GOCOMPILER_OBJS_DIR}/${GOCOMPILER_NAME}/bin/go${HOST_BINSUFFIX}"
		export GOCOMPILER_BIN_CLEAN="${GOCOMPILER_BIN}_clean"
		export GOCOMPILER_PHONY="GOCOMPILER"
		export GOCOMPILER_PHONY_CLEAN="GOCOMPILER_CLEAN"
		export GOCOMPILER_CFLAGS=
		export GOCOMPILER_LDFLAGS=""
		echo "GOCOMPILER=${GOCOMPILER}"

		#export go env
		export GOROOT=${GOCOMPILER_OBJS_DIR}/${GOCOMPILER_NAME}
		if [ "${TARGET}" = "win64" ];then
			export _GOOS="windows"
			export _GOARCH="amd64"
		elif [ "${TARGET}" = "linux64" ];then
			export _GOOS="linux"
			export _GOARCH="amd64"
		elif [ "${TARGET}" = "armv7" ];then
			export _GOOS="linux"
			export _GOARCH="arm"
		elif [ "${TARGET}" = "mac" ];then
			export _GOOS="darwin"
			export _GOARCH="amd64"
		fi
		
		#some go helper function
		export GOBIN=${GOCOMPILER_OBJS_DIR}/${GOCOMPILER_NAME}/bin
		alias GOBIN=${GOBIN}
		
		export PATH=${GOBIN}:${PATH}
		export GOEXE=go${HOST_BINSUFFIX}
		export GOEXE_CROSS=GOOS=${_GOOS} GOARCH=${_GOARCH} ${GOEXE}
		
		export GOVERSION="${GOEXE} version"
		alias GOVERSION="${GOVERSION}"
		
		export GOBUILD="${GOEXE_CROSS} build -v"
		alias GOBUILD="${GOBUILD}"
			
		export GORUN="${GOEXE} run"
		alias GORUN="${GORUN}"
			
		export GOMODINIT="${GOEXE} mod init"
		alias GOMODINIT="${GOMODINIT}"
		
		export GOTEST="${GOEXE} test"
		alias GOTEST="${GOTEST}"

		export GOTESTV="${GOEXE} test -v"
		alias GOTESTV="${GOTESTV}"

		alias GOALIAS="cd ../ && source source.sh && cd hello"
	fi
fi
