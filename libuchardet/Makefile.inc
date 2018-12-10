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

SHELL=/bin/sh

.PHONY: BINUCHARDET BINUCHARDET_CLEAN 

BINUCHARDET: ${RUNTIME_BIN}/${BINUCHARDET_BIN}
	@echo MAKE BINUCHARDET 

BINUCHARDET_CLEAN: ${BINUCHARDET_BIN}_clean
	@echo MAKE BINUCHARDET_CLEAN 

${RUNTIME_BIN}/${BINUCHARDET_BIN}:
	@echo ${RUNTIME_BIN}
	@echo $@
	$(CD) ${BINUCHARDET} && $(MAKE) -f Makefile.${BINUCHARDET_NAME} -j8 ${BINUCHARDET_BIN}

${BINUCHARDET_BIN}_clean:
	$(CD) ${BINUCHARDET} && $(MAKE) -f Makefile.${BINUCHARDET_NAME} ${BINUCHARDET_BIN}_clean
