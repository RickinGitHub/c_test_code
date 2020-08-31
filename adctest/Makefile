#common makefile header
PROJECT_DIR :=$(shell pwd)

CC 		:= arm-linux-gcc

PROM    := adc_test1
OBJ     := adc_test1.o

$(PROM) : $(OBJ)
	$(CC) -o $(PROM) $(OBJ) $(CFLAGS)
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@rm -rf $(OBJ) $(PROM) *.o
	$(info rm -rf $(OBJ) $(PROM))
