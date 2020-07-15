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

### 1. Download, Check ${NSIS_NAME}.zip
### 2. Untar, Check ${NSIS_NAME}
### 3. Config, Check ${NSIS_CHECK_CONFIG}
### 4. Make & Install, Check ${NSIS_BIN}


SHELL=/bin/sh

NSIS: ${NSIS_BIN}
	@echo MAKE $@

NSIS_CLEAN: ${NSIS_BIN_CLEAN}
	@echo MAKE $@

${NSIS_BIN}:
	$(CD) ${NSIS} && $(MAKE) -f Makefile.${NSIS_NAME} $@
	
${NSIS_BIN_CLEAN}:
	$(CD) ${NSIS} && $(MAKE) -f Makefile.${NSIS_NAME} $@

