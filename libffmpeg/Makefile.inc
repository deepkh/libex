### NetSync.tv Third-party library build scripts
### Grey Huang <deepkh@gmail.com>
### 2016

### 1. Download, Check ${LIBFFMPEG_NAME}.tar.gz
### 2. Untar, Check ${LIBFFMPEG_NAME}
### 3. Config, Check ${LIBFFMPEG_CHECK_CONFIG}
### 4. Make & Install, Check ${LIBFFMPEG_LIB}


SHELL=/bin/sh

.PHONY: LIBFFMPEG LIBFFMPEG_CLEAN

LIBFFMPEG: ${RUNTIME_LIB}/${LIBFFMPEG_LIB}
	@echo MAKE $@

LIBFFMPEG_CLEAN: ${RUNTIME_LIB}/${LIBFFMPEG_LIB}_clean
	@echo MAKE $@

${RUNTIME_LIB}/${LIBFFMPEG_LIB}:
	$(CD) ${LIBFFMPEG} && $(MAKE) -f Makefile.${LIBFFMPEG_NAME} $@
	
${RUNTIME_LIB}/${LIBFFMPEG_LIB}_clean:
	$(CD) ${LIBFFMPEG} && $(MAKE) -f Makefile.${LIBFFMPEG_NAME} $@

