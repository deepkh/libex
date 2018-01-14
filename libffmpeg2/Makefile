SHELL := /bin/bash

include $(ROOT)/mk/Makefile.pre
include $(ROOT)/mk/Makefile.post

LDFLAGS += -lavcodec$(LDLLSUFFIX) \
		   -lavfilter$(LDLLSUFFIX) \
		   -lavformat$(LDLLSUFFIX) \
		   -lavresample$(LDLLSUFFIX) \
		   -lavutil$(LDLLSUFFIX) \
		   -lswresample$(LDLLSUFFIX) \
		   -lswscale$(LDLLSUFFIX)
		  #-lpostproc$(LDLLSUFFIX)

#include $(ROOT)/mk/Makefile.dep.post

TARGET_DLL = ffmpeg.$(DLLSUFFIX)
TARGET_NAME = $(TARGET_DLL)

LIB_C_FILE = libffmpeg.c ff_buffer.c firefly_common.c
LIB_C_OBJS = $(LIB_C_FILE:.c=.o)
LIB_CXX_OBJS = $(LIB_CXX_FILE:.cpp=.o)

include $(ROOT)/mk/Makefile.common

all: $(TARGET_DLL)

install: all
	$(CP) $(TARGET_NAME) $(RUNTIME)/bin

clean: default_clean
	

