SHELL := /bin/bash

include $(ROOT)/mk/Makefile.pre
include $(ROOT)/mk/Makefile.post

#BIN_FILE =  
#BIN_OBJS = $(BIN_FILE:.c=.o)
#BIN_CXX_FILE = uchardet.cpp
#BIN_CXX_OBJS = $(BIN_CXX_FILE:.cpp=.o)

LDFLAGS +=  
LIB_FILE += $(RUNTIME)/lib/libuchardet.a
LIB_FILE_DIR = src

#APP = uchardet

include $(ROOT)/mk/Makefile.common

##########################################
#				libpkg2 
##########################################

all: $(RUNTIME)/lib/libuchardet.a

install: all

$(RUNTIME)/lib/libuchardet.a:
	$(MAKE) -C src install

clean: default_clean
	$(MAKE) -C src clean

