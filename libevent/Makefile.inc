### NetSync.tv Third-party library build scripts
### Grey Huang <deepkh@gmail.com>
### 2016

### 1. Download, Check ${LIBEVENT_NAME}.tar.gz
### 2. Untar, Check ${LIBEVENT_NAME}
### 3. Config, Check ${LIBEVENT_CHECK_CONFIG}
### 4. Make & Install, Check ${LIBEVENT_LIB}


SHELL=/bin/sh

.PHONY: LIBEVENT LIBEVENT_CLEAN

LIBEVENT: ${RUNTIME_LIB}/${LIBEVENT_LIB}
	@echo MAKE $@

LIBEVENT_CLEAN: ${RUNTIME_LIB}/${LIBEVENT_LIB}_clean
	@echo MAKE $@

${RUNTIME_LIB}/${LIBEVENT_LIB}:
	$(CD) ${LIBEVENT} && $(MAKE) -f Makefile.${LIBEVENT_NAME} $@
	
${RUNTIME_LIB}/${LIBEVENT_LIB}_clean:
	$(CD) ${LIBEVENT} && $(MAKE) -f Makefile.${LIBEVENT_NAME} $@

