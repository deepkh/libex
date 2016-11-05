include $(ROOT)/mk/Makefile.def

LIB_CXX_FILE = libfdkaac.cpp
LDFLAGS += -static

ifneq (, $(findstring mingw, $(CROSS_COMPILE1)))
	LDFLAGS += -Wl,--large-address-aware -Wl,--nxcompat -Wl,--dynamicbase
	LDFLAGS += $(RUNTIME)/lib/libfdk-aac.dll.a
	TARGET_DLL = fdkaac.$(DLLSUFFIX)
	LIB_CXX_FILE += dllmain.cpp
	TARGET_NAME = $(TARGET_DLL)
else
	LDFLAGS += -lfdk-aac
	TARGET_SOLIB = fdkaac.$(DLLSUFFIX)
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

