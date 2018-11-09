### NetSync.tv Third-party library build scripts
### Grey Huang <deepkh@gmail.com>
### 2016

### 1. Download, Check ${LIBFDKAAC_NAME}.tar.gz
### 2. Untar, Check ${LIBFDKAAC_NAME}
### 3. Config, Check ${LIBFDKAAC_CHECK_CONFIG}
### 4. Make & Install, Check ${LIBFDKAAC_LIB}


SHELL=/bin/sh

.PHONY: LIBFDKAAC LIBFDKAAC_CLEAN

LIBFDKAAC: ${RUNTIME_LIB}/${LIBFDKAAC_LIB}
	@echo MAKE $@

LIBFDKAAC_CLEAN: ${RUNTIME_LIB}/${LIBFDKAAC_LIB}_clean
	@echo MAKE $@

${RUNTIME_LIB}/${LIBFDKAAC_LIB}:
	$(CD) ${LIBFDKAAC} && $(MAKE) -f Makefile.${LIBFDKAAC_NAME} $@
	
${RUNTIME_LIB}/${LIBFDKAAC_LIB}_clean:
	$(CD) ${LIBFDKAAC} && $(MAKE) -f Makefile.${LIBFDKAAC_NAME} $@

