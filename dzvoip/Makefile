#common makefile header
PROJECT_DIR :=$(shell pwd)
SOURCE  := $(wildcard src/*.c)
OBJS    := $(patsubst %.c,%.o,$(SOURCE))

CC      := ../$(PROJECT_DIR)/buildroot/output/rockchip_rk3308_32_debug/host/usr/bin/arm-linux-gcc

INCLUDE := $(PROJECT_DIR)/include
#ALSALIB := ../$(PROJECT_DIR)/buildroot/output/rockchip_rk3308_32_debug/build/alsa-lib-1.1.5
ALGOLIB := $(PROJECT_DIR)/libs
RES     := $(PROJECT_DIR)/res

TARGET  := dzvoip

CFLAGS 	?= -L $(ALGOLIB) -lauth_rk3308_32 \
        -lduilite-rk3308_32-1.25.99-268435520-20200803171241-Alpha \
        -lduilite-rk3308_32-1.25.99-268435520-20200803171244-Alpha-unstrip \
        -lupload_rk3308_32 \
        -lasound \
        -lpthread -lm -ldl \
        -I $(INCLUDE)

$(TARGET) : $(OBJS)
	    $(CC) -o $(TARGET) $(OBJS) $(CFLAGS)

.PHONY : install everything objs clean veryclean rebuild install
install:
	cp -f $(TARGET) $(TARGET_DIR)/usr/bin/
	cp -f $(ALGOLIB)/*.so $(TARGET_DIR)/usr/lib/
	cp -rf $(RES)/* $(TARGET_DIR)/oem/

everything : $(TARGET)

all : $(TARGET)

objs : $(OBJS)

rebuild: veryclean everything

clean :
	rm -rf src/*.o
	rm $(TARGET)

veryclean : clean
	rm -rf $(TARGET)

