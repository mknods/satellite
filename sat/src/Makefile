SRC = main.c cmd.c crc.c
OBJ = $(SRC:%.c=%.o)
CFLAGS=-std=c99
hodoyoshi: $(OBJ)
	gcc $(CFLAG) -o $@ $(OBJ)
clean: 
	@rm -rf *.o hodoyoshi 

