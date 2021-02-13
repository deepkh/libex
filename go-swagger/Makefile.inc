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

### 1. Download, Check ${GOSWAGGERCLI_NAME}.tar.gz
### 2. Untar, Check ${GOSWAGGERCLI_NAME}
### 3. Config, Check ${GOSWAGGERCLI_CHECK_CONFIG}
### 4. Make & Install, Check ${GOSWAGGERCLI_BIN}


SHELL=/bin/sh

GOSWAGGERCLI: ${GOSWAGGERCLI_BIN}
	@echo MAKE $@

GOSWAGGERCLI_CLEAN: ${GOSWAGGERCLI_BIN_CLEAN}
	@echo MAKE $@

${GOSWAGGERCLI_BIN}:
	$(CD) ${GOSWAGGERCLI} && $(MAKE) -f Makefile.${GOSWAGGERCLI_NAME} $@
	
${GOSWAGGERCLI_BIN_CLEAN}:
	$(CD) ${GOSWAGGERCLI} && $(MAKE) -f Makefile.${GOSWAGGERCLI_NAME} $@

