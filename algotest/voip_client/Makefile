#common makefile header
PROJECT_DIR :=$(shell pwd)

CC 		:= arm-linux-gcc
LIBSDIR := /home/rick/share/local_rk3308/rk3308/buildroot/output/rockchip_rk3308_32_debug/build/alsa-utils-1.1.5
CFLAGS 	?= -L $(LIBSDIR) -lasound -lpthread -lm -ldl

PROM    := client_duplex
OBJ     := dz_voip_client.o

$(PROM) : $(OBJ)
	$(CC) -o $(PROM) $(OBJ) $(CFLAGS)
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@rm -rf $(OBJ) $(PROM) *.o
	$(info rm -rf $(OBJ) $(PROM))
