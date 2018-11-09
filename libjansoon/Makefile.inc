### NetSync.tv Third-party library build scripts
### Grey Huang <deepkh@gmail.com>
### 2016

### 1. Download, Check ${LIBJANSSON_NAME}.tar.gz
### 2. Untar, Check ${LIBJANSSON_NAME}
### 3. Config, Check ${LIBJANSSON_CHECK_CONFIG}
### 4. Make & Install, Check ${LIBJANSSON_LIB}


SHELL=/bin/sh

.PHONY: LIBJANSSON LIBJANSSON_CLEAN

LIBJANSSON: ${RUNTIME_LIB}/${LIBJANSSON_LIB}
	@echo $@

LIBJANSSON_CLEAN: ${RUNTIME_LIB}/${LIBJANSSON_LIB}_clean
	@echo $@

${RUNTIME_LIB}/${LIBJANSSON_LIB}:
	$(CD) ${LIBJANSSON} && $(MAKE) -f Makefile.${LIBJANSSON_NAME} $@
	
${RUNTIME_LIB}/${LIBJANSSON_LIB}_clean:
	$(CD) ${LIBJANSSON} && $(MAKE) -f Makefile.${LIBJANSSON_NAME} $@

