#--------------------------------------------------------------
#               KhoaDD2 Code for fun project
#               Esp32_Port
#--------------------------------------------------------------

BASE_PROJECT=..

LOGFILE_COMPONENT=$(BASE_PROJECT)/01_Logfile
PLATFORM_COMPONENT=$(BASE_PROJECT)/03_Platform

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

PLATFORM_LIB_TARGETS = $(LIBDIR)/static_lib_platform.a

CFLAGS +=   

CPPFLAGS := -std=c++11                                      \
            -std=gnu++11                                    \

CPPFLAGS += -I./inc                                         \
            -I $(LOGFILE_COMPONENT)/inc                     \
			-I $(PLATFORM_COMPONENT)/inc                    \

LDFLAGS := 	

LDLIBS :=   -lserial                                    \
            -L/usr/local/lib/                           \


debug: LDFLAGS += -g -O0 -DDEBUG   
debug: CPPFLAGS += -DDEBUG 
debug: all

release: all

C_SRCS =

C_OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(C_SRCS))

CPP_SRCS = src/uart_dev.cpp \
           src/uart_dev_linux.cpp \
		   src/uart_dev_wrapper.cpp \

CPP_OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(CPP_SRCS))

$(PLATFORM_LIB_TARGETS): $(C_OBJS) $(CPP_OBJS)
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

all: $(PLATFORM_LIB_TARGETS)

.PHONY: all

clean:
	@echo -e ${Yellow}Clean lib $(PLATFORM_LIB_TARGETS)${Nc}
	@rm -rf $(OBJDIR) $(LIBDIR) $(EXEDIR)

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
	@echo -e "\nC_SRCS =		\n	${C_SRCS}"
	@echo -e "\nC_OBJS =		\n	${C_OBJS}"
