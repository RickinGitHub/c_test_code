#common makefile header
PROJECT_DIR :=$(shell pwd)
CC 		:= arm-linux-gcc
LIBSDIR := /home/rick/share/local_rk3308/rk3308/buildroot/output/rockchip_rk3308_32_debug/build/alsa-utils-1.1.5
ALGOLIB := ../lib
INCLUDE := ../include
CFLAGS 	?= -L $(ALGOLIB) -lauth_rk3308_32 \
        -lduilite-rk3308_32-1.25.99-268435520-20200803171241-Alpha \
        -lduilite-rk3308_32-1.25.99-268435520-20200803171244-Alpha-unstrip \
        -lupload_rk3308_32 \
        -L $(LIBSDIR) -lasound -lpthread -lm -ldl \
        -I $(INCLUDE)
   
TARGET    := server_duplex
OBJ     := dz_voip.o \
           main.o 
$(TARGET) : $(OBJ)
	$(CC) -o $(TARGET) $(OBJ) $(CFLAGS)
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@rm -rf $(OBJ) $(TARGET) *.o
	$(info rm -rf $(OBJ) $(TARGET))
