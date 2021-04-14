#--------------------------------------------------------------
#               KhoaDD2 Code for fun project
#				Esp32_Port
#--------------------------------------------------------------

BASE_PROJECT=../..

PLATFORM_LIB=$(BASE_PROJECT)/03_Platform
LOGFILE_LIB=$(BASE_PROJECT)/01_Logfile

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

TARGETS = $(LIBDIR)/esp32_port_static_lib.a

CPPFLAGS := -I./inc                          \
            -I$(PLATFORM_LIB)/inc            \
			-I$(LOGFILE_LIB)/inc             \

CFLAGS += --std=gnu11                        \

CXXFLAGS := --std=c++11                      \

LDFLAGS := 	-Wall

LDLIBS := -lrt								\

debug: LDFLAGS += -g -O0 
debug: CPPFLAGS += -DDEBUG
debug: SUB_TARGET=debug
debug: all

release: SUB_TARGET=release
release: all

C_SRCS = src/esp32_port_timer.c \
		 src/esp32_port_uart.c

C_OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(C_SRCS))

CPP_SRCS = 	

CPP_OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(CPP_SRCS))


TARGETS_DEPS := $(PLATFORM_LIB)/$(LIBDIR)/static_lib_platform.a

$(TARGETS_DEPS):
	make -C $(PLATFORM_LIB) $(SUB_TARGET)

$(TARGETS): $(TARGETS_DEPS) $(C_OBJS) $(CPP_OBJS)
	@echo -e ${Blue}Targeting static lib $@${Nc}
	ar rcs $@ $^

$(OBJDIR)/%.o: %.cpp
	@echo -e ${Green}Compiling cpp $@ ${Nc}
	@mkdir -p $(@D)
	$(COMPILE.cc) $< -o $@

$(OBJDIR)/%.o: %.c
	@echo -e ${Green}Compiling c $@ ${Nc}
	@mkdir -p $(@D)
	$(COMPILE.c) $< -o $@

all: $(TARGETS)

clean_platform:
	@make -C $(PLATFORM_LIB) clean

clean:
	@echo -e ${Yellow}Clean lib ${TARGETS} ${Nc}
	rm -rf $(OBJDIR) $(LIBDIR) $(EXEDIR)

clean_all: clean_platform clean

.PHONY: debug
.DEFAULT_GOAL := debug

checkmake:
	@echo -e "CURDIR =		\n	${CURDIR}"
	@echo -e "\nMAKE_VERSION =	\n	${MAKE_VERSION}"
	@echo -e "\nMAKEFILE_LIST =	\n	${MAKEFILE_LIST}"
	@echo -e "\nCOMPILE.c =		\n	${COMPILE.c}"
	@echo -e "\nCOMPILE.cc =	\n	${COMPILE.cc}"
	@echo -e "\nCOMPILE.cpp =	\n	${COMPILE.cpp}"
	@echo -e "\nLINK.cc =		\n	${LINK.cc}"
	@echo -e "\nLINK.o =		\n	${LINK.o}"
	@echo -e "\nCPPFLAGS =		\n	${CPPFLAGS}"
	@echo -e "\nCFLAGS =		\n	${CFLAGS}"
	@echo -e "\nCXXFLAGS =		\n	${CXXFLAGS}"
	@echo -e "\nLDFLAGS =		\n	${LDFLAGS}"
	@echo -e "\nLDLIBS =		\n	${LDLIBS}"
