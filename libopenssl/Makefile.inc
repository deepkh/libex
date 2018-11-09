### NetSync.tv Third-party library build scripts
### Grey Huang <deepkh@gmail.com>
### 2016

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

