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

### 1. Download, Check ${LIBOPENSSL_NAME}.tar.gz
### 2. Untar, Check ${LIBOPENSSL_NAME}
### 3. Config, Check ${LIBOPENSSL_CHECK_CONFIG}
### 4. Make & Install, Check ${LIBOPENSSL_LIB}


SHELL=/bin/sh

.PHONY: LIBOPENSSL LIBOPENSSL_CLEAN

LIBOPENSSL: ${RUNTIME_LIB}/${LIBOPENSSL_LIB}
	@echo MAKE $@

LIBOPENSSL_CLEAN: ${RUNTIME_LIB}/${LIBOPENSSL_LIB}_clean
	@echo MAKE $@

${RUNTIME_LIB}/${LIBOPENSSL_LIB}:
	$(CD) ${LIBOPENSSL} && $(MAKE) -f Makefile.${LIBOPENSSL_NAME} $@
	
${RUNTIME_LIB}/${LIBOPENSSL_LIB}_clean:
	$(CD) ${LIBOPENSSL} && $(MAKE) -f Makefile.${LIBOPENSSL_NAME} $@

