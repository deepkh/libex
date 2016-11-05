### NetSync.tv Third-party library build scripts
### Grey Huang <deepkh@gmail.com>
### 2016

SHELL=/bin/sh

include $(ROOT)/Makefile.def
include $(ROOT)/Makefile.dep
include $(ROOT)/Makefile.common

all: default 

##########################################
#				libcodec
##########################################

$(RUNTIME)/bin/fdkaac.$(DLLSUFFIX): 
	$(MAKE) -C libfdkaac install; \
	$(MAKE) -C libfdkaac clean;

$(RUNTIME)/bin/x264.$(DLLSUFFIX): 
	$(MAKE) -C libx264 install; \
	$(MAKE) -C libx264 clean;

$(RUNTIME)/bin/ffmpeg.$(DLLSUFFIX): 
	$(MAKE) -C libffmpeg2 install; \
	$(MAKE) -C libffmpeg2 clean;

##########################################
#				libpkg 
##########################################

$(RUNTIME)/include/sys/queue.h:
	$(MAKE) -j4 -C $(ROOT)/libpkg $@

$(RUNTIME)/lib/libiconv.$(DLLASUFFIX): 
	$(MAKE) -j4 -C $(ROOT)/libpkg $@

$(RUNTIME)/lib/libssl.a: 
	$(MAKE) -j4 -C $(ROOT)/libpkg $@

$(RUNTIME)/lib/libevent.a: 
	$(MAKE) -j4 -C $(ROOT)/libpkg $@

$(RUNTIME)/lib/libjansson.a:
	$(MAKE) -j4 -C $(ROOT)/libpkg $@
	
$(RUNTIME)/lib/liblodepng.a:
	$(MAKE) -j4 -C $(ROOT)/libpkg $@

$(RUNTIME)/lib/libuchardet.a:
	$(MAKE) -j4 -C $(ROOT)/libpkg $@

$(RUNTIME)/lib/libx264.$(DLLASUFFIX):
	$(MAKE) -j4 -C $(ROOT)/libpkg $@

$(RUNTIME)/lib/libfdk-aac.$(DLLASUFFIX):
	$(MAKE) -j4 -C $(ROOT)/libpkg $@

$(RUNTIME)/lib/libavcodec.$(DLLASUFFIX):
	$(MAKE) -j4 -C $(ROOT)/libpkg $@

