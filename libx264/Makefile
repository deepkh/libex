SHELL := /bin/bash

include $(ROOT)/mk/Makefile.pre
include $(ROOT)/mk/Makefile.post

LDFLAGS += -lx264$(LDLLSUFFIX)

#include $(ROOT)/mk/Makefile.dep.post

TARGET_DLL = x264.$(DLLSUFFIX)
TARGET_NAME = $(TARGET_DLL)

LIB_CXX_FILE = libx264.cpp
LIB_CXX_OBJS = $(LIB_CXX_FILE:.cpp=.o)

include $(ROOT)/mk/Makefile.common

all: $(TARGET_DLL)

install: all
	$(CP) $(TARGET_NAME) $(RUNTIME)/bin

clean: default_clean
	

