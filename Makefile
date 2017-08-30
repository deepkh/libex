### NetSync.tv Third-party library build scripts
### Grey Huang <deepkh@gmail.com>
### 2016

SHELL=/bin/sh

.DEFAULT_GOAL := all

TARXZ = tar -cJvpf
DATE=`date +%Y%m%d`
GITHASH = $(shell $(ROOT)/mk/git_hash.sh)
VERSION = $(shell $(ROOT)/mk/cat_version.sh)

include $(ROOT)/mk/Makefile.pre
include $(LIBPKG)/Makefile.pre
include $(ROOT)/Makefile.pre
include $(ROOT)/mk/Makefile.post
include $(ROOT)/mk/Makefile.common

all: $(LIB_PKG_FILE) $(LIB_NS3P_FILE)

all2:
	@echo $(GITHASH)
	@echo $(VERSION)
	@echo $(LIB_PKG_FILE)
	@echo $(LDFLAGS)
	@echo $(HAVE_LIB_SSL)
	@echo $(LIBPKG)

tar:
	$(TARXZ) netsync_runtime_$(DATE)_$(VERSION)_$(GITHASH).tar.xz runtime

clean: default_clean
	$(RM) runtime build libfdkaac/fdkaac.def libffmpeg2/ffmpeg.def libx264/x264.def *.xz

