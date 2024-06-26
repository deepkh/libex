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

# BUILD_XXXXX: Make sure the build order when assigned with -jN
BUILD_BINUCHARDET:
ifneq (, $(LIBUCHARDET_PHONY))
	@$(CD) ${LIBUCHARDET} && $(MAKE) -f Makefile.inc $(LIBUCHARDET_PHONY)
endif
ifneq (, $(BINUCHARDET_PHONY))
	@$(CD) ${BINUCHARDET} && $(MAKE) -f Makefile.inc $(BINUCHARDET_PHONY)
endif

BUILD_LIBEVENT: 
ifneq (, $(LIBOPENSSL_PHONY))
	@$(CD) ${LIBOPENSSL} && $(MAKE) -f Makefile.inc $(LIBOPENSSL_PHONY)
endif
ifneq (, $(LIBEVENT_PHONY))
	@$(CD) ${LIBEVENT} && $(MAKE) -f Makefile.inc $(LIBEVENT_PHONY)
endif

EXTERNAL_LDFALGS=${LIBOPENSSL_LDFLAGS} ${LIBICONV_LDFLAGS} ${LIBEVENT_LDFLAGS2} ${LIBLODEPNG_LDFLAGS} ${LIBUCHARDET_LDFLAGS} ${LIBJANSSON_LDFLAGS}

EXTERNAL: \
		BUILD_LIBEVENT \
		$(LIBSODIUM_PHONY) \
		$(LIBZMQ_PHONY) \
		$(LIBCPPZMQ_PHONY) \
		$(LIBBOOST_PHONY) \
		$(PROTOBUF_PHONY) \
		$(LIBPROTOBUF_PHONY) \
		$(GHCFILESYSTEM_PHONY) \
		$(LIBSYS_PHONY) \
		$(LIBKLIB_PHONY) \
		$(LIBLODEPNG_PHONY) \
		$(LIBICONV_PHONY) \
		$(LIBJANSSON_PHONY) \
		BUILD_BINUCHARDET \
		$(LIBX264_PHONY) \
		$(LIBFDKAAC_PHONY) \
		$(LIBFFMPEG_PHONY) \
		$(LIBJSONCPP_PHONY) \
		$(I686W64MINGW32DLLS_PHONY) \
		$(X8664W64MINGWDLLS_PHONY) \
		$(LIBGTEST_PHONY) \
		$(GRPC_PHONY)
	@echo MAKE EXTERNAL
	@echo EXTERNAL_LDFALGS ${EXTERNAL_LDFALGS}
	
EXTERNAL_CLEAN: \
		$(LIBSODIUM_PHONY_CLEAN) \
		$(LIBZMQ_PHONY_CLEAN) \
		$(LIBCPPZMQ_PHONY_CLEAN) \
		$(LIBBOOST_PHONY_CLEAN) \
		$(GHCFILESYSTEM_PHONY_CLEAN) \
		$(LIBSYS_PHONY_CLEAN) \
		$(LIBKLIB_PHONY_CLEAN) \
		$(LIBLODEPNG_PHONY_CLEAN) \
		$(LIBICONV_PHONY_CLEAN) \
		$(LIBJANSSON_PHONY_CLEAN) \
		$(LIBUCHARDET_PHONY_CLEAN) \
		$(BINUCHARDET_PHONY_CLEAN) \
		$(LIBEVENT_PHONY_CLEAN) \
		$(LIBOPENSSL_PHONY_CLEAN) \
		$(LIBX264_PHONY_CLEAN) \
		$(LIBFDKAAC_PHONY_CLEAN) \
		$(LIBFFMPEG_PHONY_CLEAN) \
		$(LIBJSONCPP_PHONY_CLEAN) \
		$(I686W64MINGW32DLLS_PHONY_CLEAN) \
		$(PROTOCGENGO_PHONY_CLEAN) \
		$(X8664W64MINGWDLLS_PHONY_CLEAN) \
		$(LIBGTEST_PHONY_CLEAN) \
		$(GRPC_PHONY_CLEAN)
	@echo MAKE EXTERNAL_CLEAN
