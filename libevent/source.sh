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
	unset LIBEVENT_PHONY
	unset LIBEVENT_PHONY_CLEAN
	if [ "${HAVE_LIB_EVENT}" = "1" ]; then
		export LIBEVENT_NAME="libevent-2.1.8-stable"
		export LIBEVENT_SUBNAME="tar.gz"
		export LIBEVENT_LIB="libevent.${LIBSUFFIX}"
		export LIBEVENT_CONFIG_H="was_configure"
		export LIBEVENT="$1"
		export LIBEVENT_OBJS_DIR=${RUNTIME_OBJS}${LIBEVENT/${ROOT}/""}
		export LIBEVENT_PHONY="LIBEVENT"
		export LIBEVENT_PHONY_CLEAN="LIBEVENT_CLEAN"
		export LIBEVENT_CFLAGS=
		export LIBEVENT_LDFLAGS="-levent -levent_core -levent_extra -levent_openssl"
		echo "LIBEVENT=${LIBEVENT}"
	fi
fi

