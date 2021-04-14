#--------------------------------------------------------------
#               KhoaDD2 Code for fun project
#--------------------------------------------------------------

BASE_PROJECT=../..
PLATFORM_LIB=$(BASE_PROJECT)/03_Platform
LOGFILE_LIB=$(BASE_PROJECT)/01_Logfile
ESP32_LIB=$(BASE_PROJECT)/00_Esp32/Esp32_Lib
ESP32_PORT=$(BASE_PROJECT)/00_Esp32/Esp32_Port

LIBDIR ?= .libs
OBJDIR ?= .objs
EXEDIR ?= .exes

# Color for fun
LightGray='\033[0;37m'
Red='\033[0;31m'
Green='\033[0;32m'
Yellow='\033[0;33m'
Blue='\033[0;34m'
Nc='\033[0m' # No Color

# Create those dir
ISDIR := $(shell if [ ! -d $(LIBDIR) ]; then mkdir $(LIBDIR); fi;)
ISDIR := $(shell if [ ! -d $(OBJDIR) ]; then mkdir $(OBJDIR); fi;)
ISDIR := $(shell if [ ! -d $(EXEDIR) ]; then mkdir $(EXEDIR); fi;)

TARGETS = $(EXEDIR)/testapp_nw

#--------------------------------------------------------
# TOOLS BASIC CONFIG
# Note: No Need to change them
#--------------------------------------------------------
CPPFLAGS := -I inc                                      \
            -I $(LOGFILE_LIB)/inc                       \
            -I $(ESP32_PORT)/inc                        \
			-I $(ESP32_LIB)								\
            -DDEBUG                                     \

CFLAGS += 	--std=c99
		  

CXXFLAGS += --std=c++11                                 \

LDFLAGS := 	-Wall                                       \

LDLIBS :=   -lserial                                    \
            -L/usr/local/lib/                           \
			-lrt										\

debug: LDFLAGS += -g -O0
debug: SUB_TARGET=debug
debug: all

release: SUB_TARGET=release
release: all


C_SRCS = src/main.c

C_OBJS    = $(patsubst %.c,$(OBJDIR)/%.o,$(C_SRCS))

CPP_SRCS = 

CPP_OBJS    = $(patsubst %.cpp,$(OBJDIR)/%.o,$(CPP_SRCS))

TARGETS_DEPS := $(ESP32_LIB)/$(LIBDIR)/esp32_static_lib.a \
				$(ESP32_PORT)/$(LIBDIR)/esp32_port_static_lib.a \
				$(PLATFORM_LIB)/$(LIBDIR)/static_lib_platform.a \

$(TARGETS_DEPS):
	make -C $(ESP32_LIB) $(SUB_TARGET)
	make -C $(ESP32_PORT) $(SUB_TARGET)

$(TARGETS): $(C_OBJS) $(CPP_OBJS) $(TARGETS_DEPS) 
	@echo -e ${Blue}Targeting $@${Nc}
	@mkdir -p $(@D)
	$(LINK.cc) $^ $(LDLIBS) -o $@

$(OBJDIR)/%.o: %.cpp
	@echo -e ${Green}Compiling cpp $@ ${Nc}
	@mkdir -p $(@D)
	$(COMPILE.cc) -o $@ $<

$(OBJDIR)/%.o: %.c
	@echo -e ${Green}Compiling c $@ ${Nc}
	@mkdir -p $(@D)
	$(COMPILE.c) -o $@ $<

all: $(TARGETS)

.PHPNY: all test

clean_nw_lib:
	@make -C $(ESP32_LIB) clean_all

clean:
	@echo -e ${Yellow}Clean component $(TARGETS)${Nc}
	@rm -rf $(OBJDIR) $(LIBDIR) $(EXEDIR)

clean_all: clean_nw_lib clean

.PHONY:checkmake
checkmake:
	@echo -e "CURDIR =          \n  ${CURDIR}"
	@echo -e "\nMAKE_VERSION =  \n  ${MAKE_VERSION}"
	@echo -e "\nMAKEFILE_LIST = \n  ${MAKEFILE_LIST}"
	@echo -e "\nCOMPILE.c =     \n  ${COMPILE.c}"
	@echo -e "\nCOMPILE.cc =    \n  ${COMPILE.cc}"
	@echo -e "\nCOMPILE.cpp =   \n  ${COMPILE.cpp}"
	@echo -e "\nLINK.cc =       \n  ${LINK.cc}"
	@echo -e "\nLINK.o =        \n  ${LINK.o}"
	@echo -e "\nCPPFLAGS =      \n  ${CPPFLAGS}"
	@echo -e "\nCFLAGS =        \n  ${CFLAGS}"
	@echo -e "\nCXXFLAGS =      \n  ${CXXFLAGS}"
	@echo -e "\nLDFLAGS =       \n  ${LDFLAGS}"
	@echo -e "\nLDLIBS =        \n  ${LDLIBS}"
	@echo -e "\nTARGET_ARCH =   \n  ${TARGET_ARCH}"
	@echo -e "\nC_OBJS =        \n  ${C_OBJS}"
	@echo -e "\nCPP_OBJS =      \n  ${CPP_OBJS}"