### NetSync.tv Third-party library build scripts
### Grey Huang <deepkh@gmail.com>
### 2016

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

