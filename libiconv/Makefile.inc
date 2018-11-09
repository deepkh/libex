### NetSync.tv Third-party library build scripts
### Grey Huang <deepkh@gmail.com>
### 2016

### 1. Download, Check ${LIBICONV_NAME}.tar.gz
### 2. Untar, Check ${LIBICONV_NAME}
### 3. Config, Check ${LIBICONV_CHECK_CONFIG}
### 4. Make & Install, Check ${LIBICONV_LIB}


SHELL=/bin/sh

.PHONY: LIBICONV LIBICONV_CLEAN

LIBICONV: ${RUNTIME_LIB}/${LIBICONV_LIB}
	@echo MAKE $@

LIBICONV_CLEAN: ${RUNTIME_LIB}/${LIBICONV_LIB}_clean
	@echo MAKE $@

${RUNTIME_LIB}/${LIBICONV_LIB}:
	$(CD) ${LIBICONV} && $(MAKE) -f Makefile.${LIBICONV_NAME} $@
	
${RUNTIME_LIB}/${LIBICONV_LIB}_clean:
	$(CD) ${LIBICONV} && $(MAKE) -f Makefile.${LIBICONV_NAME} $@

