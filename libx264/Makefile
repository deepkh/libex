include $(ROOT)/Makefile.def

LIB_CXX_FILE = libx264.cpp

ifneq (, $(findstring mingw, $(CROSS_COMPILE1)))
	LDFLAGS += -Wl,--large-address-aware -Wl,--nxcompat -Wl,--dynamicbase
	LDFLAGS += $(RUNTIME)/lib/libx264.dll.a
	TARGET_DLL = x264.$(DLLSUFFIX)
	LIB_CXX_FILE += dllmain.cpp
	TARGET_NAME = $(TARGET_DLL)
else
	LDFLAGS += -lx264
	TARGET_SOLIB = x264.$(DLLSUFFIX)
	TARGET_NAME = $(TARGET_SOLIB)
endif

ifneq (, $(findstring mingw, $(CROSS_COMPILE1)))
endif

LIB_OBJS = $(LIB_FILE:.c=.o)
LIB_CXX_FILE += 
LIB_CXX_OBJS = $(LIB_CXX_FILE:.cpp=.o)

include $(ROOT)/Makefile.common

all: default

install: all
	$(CP) $(TARGET_NAME) $(RUNTIME)/bin
