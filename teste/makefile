CC=gcc
LIB_DIR=../lib
INC_DIR=../include

all: teste shell

teste: teste.c $(LIB_DIR)/libt2fs.a
	$(CC) -o teste teste.c -L$(LIB_DIR) -I$(INC_DIR) -lt2fs -Wall

shell: shell.c $(LIB_DIR)/libt2fs.a
	$(CC) -o shell shell.c -L$(LIB_DIR) -I$(INC_DIR) -lt2fs -Wall

clean:
	rm -rf teste shell *.o *~
