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
	unset EXTERNAL 
	unset EXTERNAL_PHONY 
	unset EXTERNAL_PHONY_CLEAN

	if [ "${HAVE_LIB_EXTERNAL}" = "1" ]; then
		export EXTERNAL="$1"
		export EXTERNAL_PHONY="EXTERNAL"
		export EXTERNAL_PHONY_CLEAN="EXTERNAL_CLEAN"
		echo "EXTERNAL=${EXTERNAL}"
		if [ -f "${EXTERNAL}/source.custom.sh" ]; then
			source "${EXTERNAL}/source.custom.sh"
		else 
			# build all packages
			source "${EXTERNAL}/platform/source.${PLATFORM}.sh"
		fi
	fi
else
	# when ${ROOT} = ${EXTERNAL}
	export HAVE_LIB_EXTERNAL=1

	# load global env
	source mk/source.sh
fi
