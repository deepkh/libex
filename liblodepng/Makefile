SHELL := /bin/bash

HAVE_DEF_SSE=
include $(ROOT)/mk/Makefile.pre
include $(ROOT)/mk/Makefile.post

PROJECT_NAME = liblodepng
TARGET_LIB = liblodepng.a

LIB_DIR = 
LIB_C_FILE = lodepng.c
LIB_C_OBJS = $(LIB_C_FILE:.c=.o)
LIB_CXX_FILE += 
LIB_CXX_OBJS = $(LIB_CXX_FILE:.cpp=.o)

include $(ROOT)/mk/Makefile.common

TARGET_INC = $(subst .c,.h,$(LIB_C_FILE)) #$(wildcard $(LIBFIREFLY)/*.h) $(wildcard $(LIBFIREFLY)/*/*.h)
TARGET_INC += $(subst .cpp,.h,$(LIB_CXX_FILE))

INSTALL_INC_DIR=$(foreach P,$(LIB_DIR),$(RUNTIME)/include/$(PROJECT_NAME)/$(P)) \
				$(RUNTIME)/include/$(PROJECT_NAME)
INSTALL_LIB_DIR=$(RUNTIME)/lib
INSTALL_ALL_DIR = $(RUNTIME) $(INSTALL_LIB_DIR)

all: $(TARGET_LIB)

install: all
	$(MKDIR) $(INSTALL_ALL_DIR)
	$(foreach P,$(INSTALL_INC_DIR),$(MKDIR) $(P);)
	$(foreach P,$(TARGET_INC),if [ -f $(P) ]; then $(CP) $(P) $(RUNTIME)/include/$(PROJECT_NAME)/$(P) ; fi;)
	$(foreach P,$(TARGET_LIB),$(CP) $(P) $(INSTALL_LIB_DIR)/$(P);)

clean: default_clean
	

