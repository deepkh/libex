SHELL := /bin/bash

include $(ROOT)/mk/Makefile.def
include $(ROOT)/mk/Makefile.dep.pre

LDFLAGS += -lfdk-aac$(LDLLSUFFIX)

include $(ROOT)/mk/Makefile.dep.post

TARGET_DLL = fdkaac.$(DLLSUFFIX)
TARGET_NAME = $(TARGET_DLL)

LIB_OBJS = $(LIB_FILE:.c=.o)
LIB_CXX_FILE = libfdkaac.cpp
LIB_CXX_OBJS = $(LIB_CXX_FILE:.cpp=.o)

include $(ROOT)/mk/Makefile.common

all: default

install: all
	$(CP) $(TARGET_NAME) $(RUNTIME)/bin

clean: default_clean
	


