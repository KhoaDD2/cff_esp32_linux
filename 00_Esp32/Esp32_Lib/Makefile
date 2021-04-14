#--------------------------------------------------------------
#               KhoaDD2 Code for fun project
#               Esp32_Port
#--------------------------------------------------------------

BASE_PROJECT=../..

ESP32_FOLDER=$(BASE_PROJECT)/00_Esp32
ESP32_PORT_FOLDER = $(ESP32_FOLDER)/Esp32_Port

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

TARGETS = $(LIBDIR)/esp32_static_lib.a

TARGETS_DEPS = $(ESP32_PORT_FOLDER)/$(LIBDIR)/esp32_port_static_lib.a

CPPFLAGS += -DASYNC_API                                         \
            -DESP32                                             \
            -DHSE_VALUE=8000000                                 \
            -DPLL_M=8                                           \
            -DESP32_BT_CENTRAL_ENABLE=1                         \
            -DESP32_WIFI_SOFTAP_ENABLE=1                        \
            -DESP32_BT_ENABLE=0                                 \
            -DESP32_WIFI_ENABLE=1                               \

CPPFLAGS += -I./inc                                             \
            -I$(BASE_PROJECT)/01_Logfile/inc                    \
            -I$(BASE_PROJECT)/00_Esp32/Esp32_Port/inc/          \
            -I3rd/Ring-Buffer/                                  \
            -Iesp32/esp32_ble/                                  \
            -Iesp32/                                            \
            -I./BLE/                                            \
            -I./WiFi/wifi/                                      \
            -I./                                                \

CFLAGS +=

CXXFLAGS +=

LDFLAGS :=  

LDLIBS :=   -lserial                                            \
            -L=/usr/local/lib/                                  \

debug: LDFLAGS += -g -O0 -DDEBUG 
debug: CFLAGS += -Wno-all                                      	\
                 -Wno-implicit-function-declaration				\
                 -Wno-int-conversion							\
                 -Wno-format
debug: SUB_TARGET=debug
debug: all

release: SUB_TARGET=release
release: all

C_SRCS = 3rd/Ring-Buffer/ringbuffer.c                           \
        BLE/ble_at.c                                            \
        bt/bt_at.c                                              \

HTTP_C_SRCS = WiFi/wifi/data/HTTPRawOut.c                       \
              WiFi/wifi/data/HTTPTextIn.c                       \
              WiFi/wifi/data/HTTPTextOut.c                      \
              WiFi/wifi/HTTPClient.c                            \
              WiFi/wifi/NM/NM.c                                 \
              WiFi/wifi/RESTClient/RESTClient_carmd.c           \
              WiFi/wifi/WiFiClient.c                            \

AT_C_SRCS =	esp32_at.c

C_OBJS = $(patsubst %.c,$(OBJDIR)/%.o,$(C_SRCS))
C_OBJS += $(patsubst %.c,$(OBJDIR)/%.o,$(HTTP_C_SRCS))
C_OBJS += $(patsubst %.c,$(OBJDIR)/%.o,$(AT_C_SRCS))

CPP_SRCS = 	

CPP_OBJS = $(patsubst src/%.cpp,$(OBJDIR)/%.o,$(CPP_SRCS))

$(TARGETS_DEPS):
	make -C ../Esp32_Port $(SUB_TARGET)

$(TARGETS): $(TARGETS_DEPS) $(C_OBJS) $(CPP_OBJS)
	@echo -e ${Blue}Targeting static lib $@ ${Nc}
	ar rcs $@ $^

$(OBJDIR)/%.o: %.cpp
	@echo -e ${Green}Compiling cpp $@ ${Nc}
	@mkdir -p $(@D)
	$(COMPILE.cc) -o $@ $<

$(OBJDIR)/%.o: %.c
	@echo -e ${Green}Compiling c $@ ${Nc}
	@mkdir -p $(@D)
	$(COMPILE.c) -o $@ $<

all: $(TARGETS_DEPS) $(TARGETS)

.PHONY: all

clean:
	@echo -e ${Yellow}Clean lib ${TARGETS} ${Nc}
	rm -rf $(OBJDIR) $(LIBDIR) $(EXEDIR)

clean_port:
	make -C ../Esp32_Port clean_all

clean_all: clean clean_port
	
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
