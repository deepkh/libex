### NetSync.tv Third-party library build scripts
### Grey Huang <deepkh@gmail.com>
### 2016

### 1. Download, Check ${LIBX264_NAME}.tar.gz
### 2. Untar, Check ${LIBX264_NAME}
### 3. Config, Check ${LIBX264_CHECK_CONFIG}
### 4. Make & Install, Check ${LIBX264_LIB}


SHELL=/bin/sh

.PHONY: LIBX264 LIBX264_CLEAN

LIBX264: ${RUNTIME_LIB}/${LIBX264_LIB}
	@echo MAKE $@

LIBX264_CLEAN: ${RUNTIME_LIB}/${LIBX264_LIB}_clean
	@echo MAKE $@

${RUNTIME_LIB}/${LIBX264_LIB}:
	$(CD) ${LIBX264} && $(MAKE) -f Makefile.${LIBX264_NAME} $@
	
${RUNTIME_LIB}/${LIBX264_LIB}_clean:
	$(CD) ${LIBX264} && $(MAKE) -f Makefile.${LIBX264_NAME} $@

