SHELL := /bin/bash

include $(ROOT)/mk/Makefile.pre
include $(ROOT)/mk/Makefile.post

PROJECT_NAME = libuchardet
LIB_DIR = 
LIB_CXX_FILE += CharDistribution.cpp \
			JpCntx.cpp \
			LangBulgarianModel.cpp \
			LangCyrillicModel.cpp \
			LangGreekModel.cpp \
			LangHebrewModel.cpp \
			LangHungarianModel.cpp \
			LangThaiModel.cpp \
			nsBig5Prober.cpp \
			nsCharSetProber.cpp \
			nsEscCharsetProber.cpp \
			nsEscSM.cpp \
			nsEUCJPProber.cpp \
			nsEUCKRProber.cpp \
			nsEUCTWProber.cpp \
			nsGB2312Prober.cpp \
			nsHebrewProber.cpp \
			nsLatin1Prober.cpp \
			nsMBCSGroupProber.cpp \
			nsMBCSSM.cpp \
			nsSBCharSetProber.cpp \
			nsSBCSGroupProber.cpp \
			nsSJISProber.cpp \
			nsUniversalDetector.cpp \
			nsUTF8Prober.cpp \
			uchardet.cpp
LIB_CXX_OBJS = $(LIB_CXX_FILE:.cpp=.o)

TARGET_LIB = libuchardet.a

include $(ROOT)/mk/Makefile.common

TARGET_INC = $(subst .c,.h,$(LIB_C_FILE))
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
	


