### NetSync.tv Third-party library build scripts
### Grey Huang <deepkh@gmail.com>
### 2016

SHELL=/bin/sh

LIBUCHARDET: ${LIBUCHARDET_LIB}
	@echo MAKE $@

LIBUCHARDET_CLEAN: ${LIBUCHARDET_LIB_CLEAN}
	@echo MAKE LIBUCHARDET_CLEAN 

${LIBUCHARDET_LIB}: 
	$(CD) ${LIBUCHARDET} && $(MAKE) -f Makefile.${LIBUCHARDET_NAME} $@

LIBUCHARDET_DEV:
	$(CD) ${LIBUCHARDET} && $(MAKE) -f Makefile.${LIBUCHARDET_NAME} ${LIBUCHARDET_LIB}
	@echo MAKE $@

${LIBUCHARDET_LIB_CLEAN}:
	$(CD) ${LIBUCHARDET} && $(MAKE) -f Makefile.${LIBUCHARDET_NAME} $@

