TOOLCHAIN_PREFIX=
U8G2_PREFIX=/usr/local
BOOST_PREFIX=

CC = $(TOOLCHAIN_PREFIX)gcc
CXX = $(TOOLCHAIN_PREFIX)g++
CXXFLAGS = 
CFLAGS = 
LIBSDL = -lSDL2main -lSDL2
LIBU8G2 = -L$(U8G2_PREFIX)/lib -l:libu8g2.a
LIBBOOST = -llibboost_system -llibboost_filesystem
LDFLAGS = -pthread
BIN_NAME = disc-emu
BUILD_DATE = $(shell date "+%Y-%m-%d %H:%M:%S")

CPP_SRC = main.cpp menu.cpp i18n.cpp i18ntools.cpp config_manager.cpp util.cpp
C_SRC = cJSON/cJSON.c
DEVICE_TYPE ?= sdl2
DEADFATTY_KEYPAD_INPUT ?= 0
ifeq ($(DEVICE_TYPE),sdl2)
    CFLAGS += -I$(U8G2_PREFIX)/include
    CXXFLAGS += --std=c++17 -I$(U8G2_PREFIX)/include
    C_SRC +=  $(shell ls ./sdl/common/*.c )
    LDFLAGS += $(LIBSDL) $(LIBU8G2) $(LIBBOOST)
else ifeq ($(DEVICE_TYPE),luckfox)
    CPP_SRC += input.cpp
    CPP_SRC += usb/network.cpp usb/usb.cpp
    CXXFLAGS += --std=c++17 -I${BOOST_PREFIX}/include -I${U8G2_PREFIX}/include -DKEYPAD_INPUT -DI2C_DISPLAY -DLUCKFOX
    ifeq ($(DEADFATTY_KEYPAD_INPUT),1)
        CXXFLAGS += -DDEADFATTY_KEYPAD_INPUT
    endif
    CFLAGS += -I${BOOST_PREFIX}/include -I${U8G2_PREFIX}/include
    LDFLAGS += -L${BOOST_PREFIX}/lib -L${U8G2_PREFIX}/lib --static
    LDFLAGS +=  -l:libu8g2arm.a -l:libu8g2fonts_gplcopyleft.a  -l:libu8g2fonts_noncommercial.a   -l:libboost_filesystem.a
endif

USB_ON ?= 1
ifeq ($(USB_ON),1)
    ifeq ($(DEVICE_TYPE),luckfox)
        CXXFLAGS+= -DUSB_ON
    endif
endif

DEFAULT_LANG ?= en
SCREEN_ROTATE ?= 0
CXXFLAGS += -DSCREEN_ROTATE=${SCREEN_ROTATE}
CXXFLAGS += -DDEFAULT_LANG=\"${DEFAULT_LANG}\" -DBUILD_DATE="\"$(BUILD_DATE)\""

C_OBJ = $(C_SRC:.c=.o)
CPP_OBJ = $(CPP_SRC:.cpp=.o)
OBJ = $(C_OBJ) $(CPP_OBJ)

# 使用 C++ 编译器链接
disc-emu: $(OBJ) 
	$(CXX) $(CXXFLAGS)  $(OBJ) -o $(BIN_NAME) $(LDFLAGS)

# 分别定义编译规则
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:	
	-rm $(OBJ) $(BIN_NAME)