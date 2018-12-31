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

### 1. Download, Check ${LIBEVENT_NAME}.tar.gz
### 2. Untar, Check ${LIBEVENT_NAME}
### 3. Config, Check ${LIBEVENT_CHECK_CONFIG}
### 4. Make & Install, Check ${LIBEVENT_LIB}


SHELL=/bin/sh

PROTOBUF: ${PROTOBUF_BIN}
	@echo MAKE $@

PROTOBUF_CLEAN: ${PROTOBUF_BIN_CLEAN}
	@echo MAKE $@

${PROTOBUF_BIN}:
	$(CD) ${PROTOBUF} && $(MAKE) -f Makefile.${PROTOBUF_NAME} $@
	
${PROTOBUF_BIN_CLEAN}:
	$(CD) ${PROTOBUF} && $(MAKE) -f Makefile.${PROTOBUF_NAME} $@


