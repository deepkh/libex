include $(ROOT)/mk/Makefile.def

CFLAGS += -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS 
LDFLAGS += -static

LIB_FILE = libffmpeg.c \
		ff_buffer.c \
		firefly_common.c
ifneq (, $(findstring mingw, $(CROSS_COMPILE1)))
	LDFLAGS += -Wl,--large-address-aware -Wl,--nxcompat -Wl,--dynamicbase
	LDFLAGS += $(RUNTIME)/lib/libavcodec.dll.a \
			   $(RUNTIME)/lib/libavfilter.dll.a \
				$(RUNTIME)/lib/libavformat.dll.a \
				$(RUNTIME)/lib/libavresample.dll.a \
				$(RUNTIME)/lib/libavutil.dll.a \
				$(RUNTIME)/lib/libswresample.dll.a \
				$(RUNTIME)/lib/libswscale.dll.a
#$(RUNTIME)/lib/libpostproc.dll.a
	TARGET_DLL = ffmpeg.$(DLLSUFFIX)
	TARGET_NAME = $(TARGET_DLL)
else
	LDFLAGS += -lavcodec -lavfilter -lavformat -lavresample -lavutil -lswresample -lswscale
	TARGET_SOLIB = ffmpeg.$(DLLSUFFIX)
	TARGET_NAME = $(TARGET_SOLIB)
endif

ifneq (, $(findstring mingw, $(CROSS_COMPILE1)))
endif

LIB_OBJS = $(LIB_FILE:.c=.o)
LIB_CXX_OBJS = $(LIB_CXX_FILE:.cpp=.o)

include $(ROOT)/Makefile.common

all: default

install: all
	$(CP) $(TARGET_NAME) $(RUNTIME)/bin
