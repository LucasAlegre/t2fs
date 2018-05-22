#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apidisk.h"
#include "bitmap2.h"
#include "t2fs_aux.h"
#include "t2fs.h"

BOOL initalizedT2fs = FALSE;

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

	return 0;
}

void initializeOpenFiles(){
	int i;
	for(i = 0; i < MAX_OPEN_FILES; i++){
		openFiles[i].record.TypeVal = TYPEVAL_INVALIDO;
	}
}

int getInode(DWORD inodeNumber, struct t2fs_inode *inode){

	int inodeSector = inodeAreaStartSector + inodeNumber/INODE_PER_SECTOR;
	unsigned char buffer[SECTOR_SIZE];

	if(getBitmap2(BITMAP_INODE, inodeNumber) == 0){  // If inode is free return -1
		return -1;
	}

	if(read_sector(inodeSector, buffer) != 0){
		printf("Error: Failed reading inode sector!\n");
		return -1;
	}

	int inode_byte_start = (inodeNumber % INODE_PER_SECTOR)*INODE_SIZE;
	inode->blocksFileSize = *((DWORD*) (buffer + inode_byte_start + 0));
	inode->bytesFileSize = *((DWORD*) (buffer + inode_byte_start + 4));
	inode->dataPtr[0] = *((DWORD*) (buffer + inode_byte_start + 8));
	inode->dataPtr[1] = *((DWORD*) (buffer + inode_byte_start + 12));
	inode->singleIndPtr = *((DWORD*) (buffer + inode_byte_start + 16));
	inode->doubleIndPtr = *((DWORD*) (buffer + inode_byte_start + 20));   
	inode->reservado[0] = *((DWORD*) (buffer + inode_byte_start + 24));
	inode->reservado[1] = *((DWORD*) (buffer + inode_byte_start + 28));

	if(DEBUG && 0){
		printf("Inode blocksFileSize: %d\n", inode->blocksFileSize);
		printf("Inode bytesFileSize: %d\n", inode->bytesFileSize);
		printf("Inode dataPtr[0]: %d\n", inode->dataPtr[0]);
	}

	return 0;
}

// Get all records of the block given
int getRecords(DWORD blockNumber, Record *records){
	unsigned char buffer[SECTOR_SIZE];
	int i, j, c;

	for(i = 0; i < superBlock.blockSize; i++){ // For all sector of block
		int sectorNumber = blockNumber*superBlock.blockSize + i;
		read_sector(sectorNumber, buffer);
		for(j = 0; j < RECORD_PER_SECTOR; j++){  // For all record of sector
			records[j + i*RECORD_PER_SECTOR].TypeVal = buffer[j*RECORD_SIZE];
			for(c = 0; c < 59; c++){
				records[j + i*RECORD_PER_SECTOR].name[c] = buffer[1 + c + j*RECORD_SIZE];
			}
			records[j + i*RECORD_PER_SECTOR].inodeNumber = *((DWORD*)(buffer + 60 + j*RECORD_SIZE));
		}
	}
	return 0;
}

void initializeT2fs(){
	if(initalizedT2fs)
		return;

	if(readSuperBlock() != 0){
		printf("Error: SuperBlock not read!\n");
	}
	initializeOpenFiles();

	dataAreaStartBlock = superBlock.superblockSize + superBlock.freeBlocksBitmapSize + superBlock.freeInodeBitmapSize + superBlock.inodeAreaSize;
	inodeAreaStartSector = superBlock.superblockSize*superBlock.blockSize + superBlock.freeBlocksBitmapSize*superBlock.blockSize + superBlock.freeInodeBitmapSize*superBlock.blockSize;

	currentDirInode = ROOT_INODE;
	strcpy(currentPath, "/\0");

	initalizedT2fs = TRUE;

	if(DEBUG){
		printf("Id: %s\n", superBlock.id);
		printf("Version: %d\n", superBlock.version);
		printf("SuperBlock Size (Blocks): %d\n", superBlock.superblockSize);
		printf("freeBlocksBitmapSize (Blocks): %d\n", superBlock.freeBlocksBitmapSize);
		printf("freeInodeBitmapSize (Blocks): %d\n", superBlock.freeInodeBitmapSize);
		printf("inodeAreaSize (Blocks): %d\n", superBlock.inodeAreaSize);
		printf("blockSize (Sectors): %d\n", superBlock.blockSize);
		printf("diskSize (Blocks): %d\n", superBlock.diskSize);

		printf("InodeSize (Bytes): %d\n", sizeof(struct t2fs_inode));
		printf("Number of inodes: %d\n", ((superBlock.inodeAreaSize*superBlock.blockSize)*SECTOR_SIZE)/INODE_SIZE);
		printf("Number of Data Blocks: %d\n", superBlock.diskSize - dataAreaStartBlock);
		printf("Data Area Start Block: %d\n", dataAreaStartBlock);
		printf("Inode Area Start Sector: %d\n", inodeAreaStartSector);

		printf("First Free Inode: %d\n", searchBitmap2(BITMAP_INODE, 0));
		printf("Current Path: %s\n", currentPath);

		FILE2 file = open2("file3");
		printf("%s\n", openFiles[file].record.name);
	}
}

// Finde the file "filename" in one of the entries of the block given,
// if found put it on *record
int getRecordFromEntryBlock(DWORD blockNumber, char *filename, Record *record){
	Record records[RECORD_PER_SECTOR*superBlock.blockSize];
	getRecords(blockNumber, records);
	int i;
	for(i = 0; i < RECORD_PER_SECTOR*superBlock.blockSize; i++){
		if(records[i].TypeVal != TYPEVAL_INVALIDO && strcmp(records[i].name, filename) == 0){
			*record = records[i];
			return 0;
		}
	}
	return -1; // File not found
}

// Return first available position in the openFiles array
// -1 if full
FILE2 getFreeFileHandle(){
	FILE2 freeHandle;
	for(freeHandle = 0; freeHandle < MAX_OPEN_FILES; freeHandle++){
		if(openFiles[freeHandle].record.TypeVal == TYPEVAL_INVALIDO)
			return freeHandle;
	}
	return -1;
}

// Retorna todos os pointeiros em um bloco
int getPointers(DWORD blockNumber, DWORD *pointers){
	unsigned char buffer[SECTOR_SIZE];
	int i, j;

	for(i = 0; i < superBlock.blockSize; i++){ // For all sector of block
		int sectorNumber = blockNumber*superBlock.blockSize + i;
		read_sector(sectorNumber, buffer);
		for(j = 0; j < PTR_PER_SECTOR; j++){  // For all record of sector
			pointers[j + i*PTR_PER_SECTOR] = *((DWORD*)(buffer + j*PTR_SIZE));
		}
	}
	return 0;
}