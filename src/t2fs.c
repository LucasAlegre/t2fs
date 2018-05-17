#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apidisk.h"
#include "bitmap2.h"
#include "t2fs.h"
#include "constants.h"

// Declarar aqui as vaŕiáveis globais
struct t2fs_superbloco superBlock;

int readSuperBlock(){

	unsigned char buffer[SECTOR_SIZE];

	if(read_sector(0, buffer) != 0){
		printf("Error: Failed reading sector 0!\n");
		return -1;
	}

	strncpy(superBlock.id, (char*)buffer, 4);
	superBlock.version = *( (DWORD*)(buffer + 4) );
	superBlock.superblockSize = *( (WORD*)(buffer + 6) );
	superBlock.freeBlocksBitmapSize = *( (WORD*)(buffer + 8) ); 
	superBlock.freeInodeBitmapSize = *( (WORD*)(buffer + 10) );
	superBlock.inodeAreaSize = *( (WORD*)(buffer + 12) );
	superBlock.blockSize = *( (WORD*)(buffer + 14) );
	superBlock.diskSize = *( (DWORD*)(buffer + 16) );

	if(DEBUG){
		printf("Id: %s\n", superBlock.id);
		printf("Version: %d\n", superBlock.version);
		printf("SuperBlock Size: %d\n", superBlock.superblockSize);
		printf("freeBlocksBitmapSize: %d\n", superBlock.freeBlocksBitmapSize);
		printf("freeInodeBitmapSize: %d\n", superBlock.freeInodeBitmapSize);
		printf("inodeAreaSize: %d\n", superBlock.inodeAreaSize);
		printf("blockSize: %d\n", superBlock.blockSize);
		printf("diskSize: %d\n", superBlock.diskSize);
	}

	return 0;

}

int identify2 (char *name, int size){

	char *group = "Lucas Nunes Alegre 00274693\nAline Weber 00274720\nLucas Sonntag Hagen 00274698\0";
	if(size < strlen(group)){
		printf("Size given is not sufficient to copy the whole string!\n");
		return -1;
	}
	strncpy(name, group, strlen(group)+1);

	return 0;
}


FILE2 create2 (char *filename){
	return -1;
}


int delete2 (char *filename){
	return -1;
}


FILE2 open2 (char *filename){
	return -1;
}


int close2 (FILE2 handle){
	return -1;
}


int read2 (FILE2 handle, char *buffer, int size){
	return -1;
}


int write2 (FILE2 handle, char *buffer, int size){
	return -1;
}


int truncate2 (FILE2 handle){
	return -1;
}

int seek2 (FILE2 handle, DWORD offset){
	return -1;
}


int mkdir2 (char *pathname){
	return -1;
}


int rmdir2 (char *pathname){
	return -1;
}


int chdir2 (char *pathname){
	return -1;
}


int getcwd2 (char *pathname, int size){
	return -1;
}

DIR2 opendir2 (char *pathname){
	return -1;
}

int readdir2 (DIR2 handle, DIRENT2 *dentry){
	return -1;
}

int closedir2 (DIR2 handle){
	return -1;
}