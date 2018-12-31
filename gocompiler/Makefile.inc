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

### 1. Download, Check ${GOCOMPILER_NAME}.tar.gz
### 2. Untar, Check ${GOCOMPILER_NAME}
### 3. Config, Check ${GOCOMPILER_CHECK_CONFIG}
### 4. Make & Install, Check ${GOCOMPILER_BIN}


SHELL=/bin/sh

GOCOMPILER: ${GOCOMPILER_BIN}
	@echo MAKE $@

GOCOMPILER_CLEAN: ${GOCOMPILER_BIN_CLEAN}
	@echo MAKE $@

${GOCOMPILER_BIN}:
	$(CD) ${GOCOMPILER} && $(MAKE) -f Makefile.${GOCOMPILER_NAME} $@
	
${GOCOMPILER_BIN_CLEAN}:
	$(CD) ${GOCOMPILER} && $(MAKE) -f Makefile.${GOCOMPILER_NAME} $@

