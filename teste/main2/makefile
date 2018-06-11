#
# Makefile para programas exemplo
#

CC=gcc
LIB_DIR=../../lib
INC_DIR =../../include

all: main

main: main.c $(LIB_DIR)/libt2fs.a
	$(CC) -o main main.c -L$(LIB_DIR) -I$(INC_DIR) -lt2fs -Wall

clean:
	rm -rf main *.o *~