#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apidisk.h"
#include "bitmap2.h"
#include "t2fs_aux.h"
#include "t2fs.h"

BOOL initalizedT2fs = FALSE;

void initializeT2fs(){
	if(initalizedT2fs)
		return;

	if(readSuperBlock() != 0){
		printf("Error: SuperBlock not read!\n");
	}
	initializeOpenFiles();
	initializeOpenDirs();

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

		Inode inode;
		getInodeFromInodeNumber(0, &inode);
		printf("Tamanho bytes: %d\n", inode.bytesFileSize);
		printf("Tamanho blocos: %d\n", inode.blocksFileSize);

		/*
		char *name = "dir1/dir11/../dir11/file111\0";
		int h = open2(name);
		if (isFileHandleValid(h)){
			printf("%s\n", openFiles[h].record.name);
			printf("%d\n", openFiles[h].record.inodeNumber);
			printf("%d\n", openFiles[h].record.TypeVal);
		}*/

		int h = create2("/dir1/dir11/batata\0");
		printf("%d\n", h);
		if (isFileHandleValid(h)){
			printf("%s\n", openFiles[h].record.name);
			printf("%d\n", openFiles[h].record.inodeNumber);
			printf("%d\n", openFiles[h].record.TypeVal);
		}
		close2(h);
		if(delete2("/dir1/dir11/batata\0") == 0){
			h = open2("/dir1/dir11/batata\0");
			printf("%d\n", h);
		}
	}
}

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
		openFiles[i].record.inodeNumber = INVALID_PTR;
	}
}

void initializeOpenDirs(){
	int i;
	for(i = 0; i < MAX_OPEN_DIR; i++){
		openDirs[i].record.TypeVal = TYPEVAL_INVALIDO;
		openDirs[i].record.inodeNumber = INVALID_PTR;
	}
}

int getInodeFromInodeNumber(DWORD inodeNumber, struct t2fs_inode *inode){

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

int getRecordsFromEntryBlock(DWORD blockNumber, Record *records){
	unsigned char buffer[SECTOR_SIZE];
	int i, j, c;

	for(i = 0; i < superBlock.blockSize; i++){ // For all sector of block
		int sectorNumber = blockNumber*BLOCK_SIZE + i;
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

int getRecordFromBlockWithNumber(DWORD blockNumber, int recordNumber, Record *record){
	Record records[RECORD_PER_BLOCK];
	if(blockNumber == INVALID_PTR || getRecordsFromEntryBlock(blockNumber, records) != 0) {
		return -1; // FILE NOT FOUND
	}
	
	*record = records[recordNumber];
	return 0;
}

int getRecordFromEntryBlock(DWORD blockNumber, char *filename, Record *record){

	Record records[RECORD_PER_SECTOR*BLOCK_SIZE];
	getRecordsFromEntryBlock(blockNumber, records);

	int i;
	for(i = 0; i < RECORD_PER_SECTOR*superBlock.blockSize; i++){
		if(records[i].TypeVal != TYPEVAL_INVALIDO && strcmp(records[i].name, filename) == 0){
			*record = records[i];
			return 0;
		}
	}
	return -1; // File not found
}

FILE2 getFreeFileHandle(){
	FILE2 freeHandle;
	for(freeHandle = 0; freeHandle < MAX_OPEN_FILES; freeHandle++){
		if(openFiles[freeHandle].record.TypeVal == TYPEVAL_INVALIDO)
			return freeHandle;
	}
	return -1;
}

DIR2 getFreeDirHandle(){
	DIR2 freeHandle;
	for(freeHandle = 0; freeHandle < MAX_OPEN_DIR; freeHandle++){
		if(openDirs[freeHandle].record.TypeVal == TYPEVAL_INVALIDO)
			return freeHandle;
	}
	return -1;
}

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

int getRecordFromDir(Inode dirInode, char *filename, Record *recordOut){
	int i, j;
	// Search on direct pointers
	Record record;
	DWORD pointers[PTR_PER_SECTOR*BLOCK_SIZE];
	DWORD doublePointers[PTR_PER_SECTOR*BLOCK_SIZE];

	for(i = 0; i < 2; i++){
		if(dirInode.dataPtr[i] != INVALID_PTR){
			if(getRecordFromEntryBlock(dirInode.dataPtr[i], filename, &record) == 0){
				*recordOut = record;
				return 0;
			}
		}
	}
	// Search on simple indirection
	if(dirInode.singleIndPtr != INVALID_PTR){
		getPointers(dirInode.singleIndPtr, pointers);
		for(i = 0; i < PTR_PER_SECTOR*BLOCK_SIZE; i++){
			if(pointers[i] != INVALID_PTR){
				if(getRecordFromEntryBlock(pointers[i], filename, &record) == 0){
					*recordOut = record;
					return 0;
				}
			}
		}
	}
	// Search on double indirection
	if(dirInode.doubleIndPtr != INVALID_PTR){	
		getPointers(dirInode.doubleIndPtr, doublePointers);
		for(i = 0; i < PTR_PER_SECTOR*BLOCK_SIZE; i++){
			if(doublePointers[i] != INVALID_PTR){
				getPointers(doublePointers[i], pointers);
				for(j = 0; j < PTR_PER_SECTOR*BLOCK_SIZE; j++){
					if(pointers[j] != INVALID_PTR){
						if(getRecordFromEntryBlock(pointers[j], filename, &record) == 0){
							*recordOut = record;
							return 0;
						}
					}
				}
			}
		}
	}

	return -1;
}

int getRecordFromPath(char *pathname, Record *recordOut){

	Inode dirInode;
	char filename[MAX_FILE_NAME_SIZE+1];

	if(getLastDirInode(pathname, &dirInode) != 0)
		return -1;

	getFilenameFromPath(pathname, filename);
	if(getRecordFromDir(dirInode, filename, recordOut) != 0)
		return -1;
	
	return 0;
}

int getRecordFromNumber(DWORD inodeNumber, int pointer, Record *record) {
	Inode inode;

	int block = pointer / RECORD_PER_BLOCK;
	int recordNumber = pointer % RECORD_PER_BLOCK;

	DWORD pointers[PTR_PER_SECTOR*superBlock.blockSize];
	DWORD blockAddr = INVALID_PTR;

	if(getInodeFromInodeNumber(inodeNumber, &inode) != 0) {
		return -1;
	}

	if(pointer >= inode.bytesFileSize/RECORD_SIZE)
		return -2; 									// END OF FILE

	if (block == 0 || block == 1) { 				// DIRECT DATA POINTERS

		blockAddr = inode.dataPtr[block];

	} else if((block - 2) < POINTERS_PER_BLOCK) {	// SIMPLE INDIRECTION

		if(inode.singleIndPtr == INVALID_PTR)
			return -1;

		getPointers(inode.singleIndPtr, pointers);

		block -= 2;
		blockAddr = pointers[block];

	} else {										// DOUBLE INDIRECTION

		block = (block - 2) - POINTERS_PER_BLOCK;
		int pointerBlock = block / POINTERS_PER_BLOCK;

		if(inode.doubleIndPtr == INVALID_PTR)
			return -1;

		getPointers(inode.doubleIndPtr, pointers);

		if(pointerBlock >= POINTERS_PER_BLOCK || pointers[pointerBlock] == INVALID_PTR)
			return -1;

		getPointers(pointers[pointerBlock], pointers);

		blockAddr = pointers[block];
	}

	if(blockAddr == INVALID_PTR)
		return -1;

	return getRecordFromBlockWithNumber(blockAddr, recordNumber, record);
}

int getLastDirInode(char *pathname, Inode *inode){
	int isAbsolute = (*pathname == '/');

	int count = 0;

	char *path;
	char split[] = "/";
	char *token;

	Inode parent;
	Record record;

	path = malloc(sizeof(char) * (strlen(pathname) + 1));
	memset(path, '\0', sizeof(path));
	strncpy(path, pathname, strlen(pathname));

	if(isAbsolute) {	// ABSOLUTE PATH
		record.inodeNumber = 0;
	} else {			// RELATIVE PATH
		record.inodeNumber = currentDirInode;
	}

	token = strtok(path, split);
	
	while(token != NULL) {

		if(getInodeFromInodeNumber(record.inodeNumber, &parent) != 0) {
			free(path);
			return -1;
		}
		
		if(getRecordFromDir(parent, token, &record) != 0) {
			if(strtok(NULL, split) == NULL){  // Era o nome do arquivo -> O arquivo final não precisa existir desde que o lastDir exista
				*inode = parent;
				free(path);
				return 0;
			}
			free(path);
			return -2;
		}

		token = strtok(NULL, split);
		count++;
	}

	if(count == 0) { // TRATAR CASO ESPECIAL (pathname vazio ou /)
		if(getInodeFromInodeNumber(record.inodeNumber, &parent) != 0 ||
			getRecordFromDir(parent, "..", &record) != 0 ||
			getInodeFromInodeNumber(record.inodeNumber, &parent) != 0) {

			free(path);
			return -3;
		}
	}
	
	free(path);
	*inode = parent;
	return 0;
}

BOOL isDirEmpty(Inode *dirInode){return FALSE;}

void removeAllDataFromInode(int inodeNumber){
	Inode inode;
	DWORD pointers[PTR_PER_SECTOR*BLOCK_SIZE];
	DWORD doublePointers[PTR_PER_SECTOR*BLOCK_SIZE];
	int i, j;

	getInodeFromInodeNumber(inodeNumber, &inode);

	// Libera blocos diretos
	for(i = 0; i < 2; i++){
		if(inode.dataPtr[i] != INVALID_PTR){
			setBitmap2(BITMAP_DADOS, inode.dataPtr[i], 0);
		}
	}

 	// Libera indireção simples
	if(inode.singleIndPtr != INVALID_PTR){
		getPointers(inode.singleIndPtr, pointers);
		for(i = 0; i < PTR_PER_SECTOR*BLOCK_SIZE; i++){
			if(pointers[i] != INVALID_PTR){
				setBitmap2(BITMAP_DADOS, pointers[i], 0);
			}
		}
		setBitmap2(BITMAP_DADOS, inode.singleIndPtr, 0);
	}

	// Libera indireção dupla
	if(inode.doubleIndPtr != INVALID_PTR){
		getPointers(inode.doubleIndPtr, doublePointers);
		for(i = 0; i < PTR_PER_SECTOR*BLOCK_SIZE; i++){
			if(doublePointers[i] != INVALID_PTR){
				getPointers(doublePointers[i], pointers);
				for(j = 0; j < PTR_PER_SECTOR*BLOCK_SIZE; j++){
					if(pointers[j] != INVALID_PTR){
						setBitmap2(BITMAP_DADOS, pointers[j], 0);
					}
				}
				setBitmap2(BITMAP_DADOS, doublePointers[i], 0);
			}
		}
		setBitmap2(BITMAP_DADOS, inode.doubleIndPtr, 0);
	}
}

int updateRecord(Inode dirInode, Record recordToChange, BYTE typeVal){

	int i, j, p;
	Record records[RECORD_PER_SECTOR*BLOCK_SIZE];
	DWORD pointers[PTR_PER_SECTOR*BLOCK_SIZE];
	DWORD doublePointers[PTR_PER_SECTOR*BLOCK_SIZE];

	for(p = 0; p < 2; p++){
		if(dirInode.dataPtr[p] != INVALID_PTR){
			getRecordsFromEntryBlock(dirInode.dataPtr[p], records);
			for(i = 0; i < RECORD_PER_SECTOR*BLOCK_SIZE; i++){
				if(records[i].TypeVal == typeVal && strcmp(records[i].name, recordToChange.name) == 0){
					if(writeRecordOnDir(dirInode.dataPtr[p], recordToChange, i) == 0){
						return 0;
					}
				}
			}
		}
	}

	if(dirInode.singleIndPtr != INVALID_PTR){
		getPointers(dirInode.singleIndPtr, pointers);
		for(i = 0; i < PTR_PER_SECTOR*BLOCK_SIZE; i++){
			if(pointers[i] != INVALID_PTR){
				getRecordsFromEntryBlock(pointers[i], records);
				for(j = 0; j < RECORD_PER_SECTOR*BLOCK_SIZE; j++){
					if(records[i].TypeVal == typeVal && strcmp(records[i].name, recordToChange.name) == 0){
						if(writeRecordOnDir(pointers[i], recordToChange, i) == 0){
							return 0;
						}
					}
				}
			}
		}
	}

	if(dirInode.doubleIndPtr != INVALID_PTR){
		getPointers(dirInode.doubleIndPtr, doublePointers);
		for(i = 0; i < PTR_PER_SECTOR*BLOCK_SIZE; i++){
			if(doublePointers[i] != INVALID_PTR){
				getPointers(doublePointers[i], pointers);
				for(j = 0; j < PTR_PER_SECTOR*BLOCK_SIZE; j++){
					if(pointers[j] != INVALID_PTR){
						getRecordsFromEntryBlock(pointers[j], records);
						for(p = 0; p < RECORD_PER_SECTOR*BLOCK_SIZE; p++){
							if(records[p].TypeVal == typeVal && strcmp(records[p].name, recordToChange.name) == 0){
								if(writeRecordOnDir(pointers[j], recordToChange, p) == 0){
									return 0;
								}
							}
						}
					}
				}
			}
		}
	}

	return -1;
}

void initNewDirInode(DWORD inodeNumber){
/*
	marcar como ocupado no bitmap
	inicializar um bloco de dados
	criar entradas ./ e ../
*/
}

int addRecordOnDir(Inode dirInode, Record record){
	int i, j;
	Record records[RECORD_PER_SECTOR*BLOCK_SIZE];

	getRecordsFromEntryBlock(dirInode.dataPtr[0], records);
	for(i = 0; i < RECORD_PER_SECTOR*BLOCK_SIZE; i++){
		if(records[i].TypeVal == TYPEVAL_INVALIDO){
			if(writeRecordOnDir(dirInode.dataPtr[0], record, i) == 0){
				return 0;
			}
		}
	}

	if(dirInode.dataPtr[1] == INVALID_PTR){
		dirInode.dataPtr[1] = initNewEntryBlock();
		if(dirInode.dataPtr[1] == -1)
			return -1;
	}
	getRecordsFromEntryBlock(dirInode.dataPtr[1], records);
	for(i = 0; i < RECORD_PER_SECTOR*BLOCK_SIZE; i++){
		if(records[i].TypeVal == TYPEVAL_INVALIDO){
			if(writeRecordOnDir(dirInode.dataPtr[1], record, i) == 0)
				return 0;
		}
	}

	
	if(dirInode.singleIndPtr == INVALID_PTR){
		dirInode.singleIndPtr = initNewPointerBlock();
	}
	DWORD pointers[PTR_PER_SECTOR*BLOCK_SIZE];
	getPointers(dirInode.singleIndPtr, pointers);
	for(i = 0; i < PTR_PER_SECTOR*BLOCK_SIZE; i++){
		if(pointers[i] != INVALID_PTR){
			getRecordsFromEntryBlock(pointers[i], records);
			for(j = 0; j < RECORD_PER_SECTOR*BLOCK_SIZE; j++){
				if(records[i].TypeVal == TYPEVAL_INVALIDO){
					if(writeRecordOnDir(pointers[i], record, j) == 0)
						return 0;
				}
			}
		}
		else{
			pointers[i] = initNewEntryBlock();
			writeRecordOnDir(pointers[i], record, 0);
			return 0;
		}
	}


	// FALTA DUPLA INDIREÇÃO

	return -1;
}

int writeRecordOnDir(DWORD blockNum, Record record, int recordNum){
	unsigned char buffer[SECTOR_SIZE];
	int i;
	int sector = blockNum*BLOCK_SIZE + (recordNum*RECORD_SIZE)/(SECTOR_SIZE);
	int byte_start = (recordNum % RECORD_PER_SECTOR)*RECORD_SIZE;
	if(read_sector(sector, buffer) != 0){
		printf("Erro leitura do setor %d\n", sector);
		return -1;
	}

	buffer[byte_start] = record.TypeVal;
	for(i = 0; i < 59; i++){
		buffer[1 + i + byte_start] = record.name[i];
	}
	writeDwordOnBuffer(buffer, byte_start + 60, record.inodeNumber);
	if(write_sector(sector, buffer) != 0){
		return -1;
	}

	return 0;
}

int initNewEntryBlock(){
	Record emptyRecord;
	int i;
	emptyRecord.TypeVal = TYPEVAL_INVALIDO;
	emptyRecord.inodeNumber = INVALID_PTR;
	int blockNum = searchBitmap2(BITMAP_DADOS, 0);
	if(blockNum <= 0)
		return -1;
	for(i = 0; i < RECORD_PER_SECTOR*BLOCK_SIZE; i++){
		if(writeRecordOnDir(blockNum, emptyRecord, i) != 0)
			return -1;
	}
	setBitmap2(BITMAP_DADOS, blockNum, 1);
	return blockNum;
}

int initNewPointerBlock(){
	DWORD pointer = INVALID_PTR;
	int i;
	unsigned char buffer[SECTOR_SIZE];
	for(i = 0; i < PTR_PER_SECTOR; i++){
		writeDwordOnBuffer(buffer, i*PTR_SIZE, pointer);
	}
	int blockNum = searchBitmap2(BITMAP_DADOS, 0);
	if(blockNum <= 0)
		return -1;
	for(i = 0; i < BLOCK_SIZE; i++){
		if(write_sector(blockNum*BLOCK_SIZE + i, buffer) != 0)
			return -1;
	}
	setBitmap2(BITMAP_DADOS, blockNum, 1);
	return blockNum;
}

void printError(char *error) {
	if(DEBUG) {
		printf("[ERRO] %s\n", error);
	}
}

BOOL isDirHandleValid(DIR2 handle){
	if(handle < 0 || handle >= MAX_OPEN_DIR || openDirs[handle].record.TypeVal != TYPEVAL_DIRETORIO)
		return FALSE;
	else
		return TRUE;
}

BOOL isFileHandleValid(FILE2 handle){
	if(handle < 0 || handle >= MAX_OPEN_FILES || openFiles[handle].record.TypeVal != TYPEVAL_REGULAR)
		return FALSE;
	else
		return TRUE;
}

int initNewFileInode(){
	DWORD inodeNumber = searchBitmap2(BITMAP_INODE, 0);
	if(inodeNumber <= 0)
		return -1;
	Inode inode;
	inode.blocksFileSize = 1;
	inode.bytesFileSize = 0;
	inode.dataPtr[1] = INVALID_PTR;
	inode.singleIndPtr = INVALID_PTR;
	inode.doubleIndPtr = INVALID_PTR;
	int blockNum = searchBitmap2(BITMAP_DADOS, 0);
	if(blockNum <= 0)
		return -1;
	setBitmap2(BITMAP_DADOS, blockNum, 1);
	inode.dataPtr[0] = blockNum;
	setBitmap2(BITMAP_INODE, inodeNumber, 1);

	if(writeInodeOnDisk(inode, inodeNumber) != 0)
		return -1;

	return inodeNumber;
}

void writeDwordOnBuffer(unsigned char *buffer, int start, DWORD dword){
	unsigned char *aux;
	aux = (unsigned char*)&dword;
	int i;
	for(i = 0; i < 4; i++)
		buffer[start + i] = aux[i];
}

int writeInodeOnDisk(Inode inode, int inodeNumber){
	int inodeSector = inodeAreaStartSector + inodeNumber/INODE_PER_SECTOR;
	unsigned char buffer[SECTOR_SIZE];

	if(read_sector(inodeSector, buffer) != 0)
		return -1;

	int inode_byte_start = (inodeNumber % INODE_PER_SECTOR)*INODE_SIZE;
	writeDwordOnBuffer(buffer, inode_byte_start + 0, inode.blocksFileSize);
	writeDwordOnBuffer(buffer, inode_byte_start + 4, inode.bytesFileSize);
	writeDwordOnBuffer(buffer, inode_byte_start + 8, inode.dataPtr[0]);
	writeDwordOnBuffer(buffer, inode_byte_start + 12, inode.dataPtr[1]);
	writeDwordOnBuffer(buffer, inode_byte_start + 16, inode.singleIndPtr);
	writeDwordOnBuffer(buffer, inode_byte_start + 20, inode.doubleIndPtr);

	if(write_sector(inodeSector, buffer) != 0)
		return -1;

	return 0;
}

void fixPath(char* path) {
    char originalPath[MAX_FILE_NAME_SIZE+1];
    strcpy(originalPath, path);
    *path = '\0';
    char split[] = "/";
    char* token;
    char* c;
    
    token = strtok(originalPath, split);
   
    while( token != NULL ) {
        if(strcmp(token, "..") == 0) {
            c = strrchr(path, '/');
            if(c != NULL)
                *c = '\0';
                
        } else if(strcmp(token, "") != 0 && strcmp(token, ".") != 0) {
            strcat(path, "/");
            strcat(path, token);
        }
        
        token = strtok(NULL, split);
    }
    strcat(path, "/");
}

void getFilenameFromPath(char *pathname, char *filename){
	char *path;
	char *aux;
	path = malloc(sizeof(char) * (strlen(pathname) + 1));
	memset(path, '\0', sizeof(path));
	strncpy(path, pathname, strlen(pathname));

	aux = strtok(path, "/");
	strcpy(filename, aux);

	while(aux != NULL){
		strcpy(filename, aux);
		aux = strtok(NULL, "/");
	}
	free(path);
}

BOOL isFileOpen(int inodeNumber){
	int i;
	for(i = 0; i < MAX_OPEN_FILES; i++){
		if(openFiles[i].record.inodeNumber == inodeNumber)
			return TRUE;
	}
	return FALSE;
}

BOOL isDirOpen(int inodeNumber){
	int i;
	for(i = 0; i < MAX_OPEN_DIR; i++){
		if(openDirs[i].record.inodeNumber == inodeNumber)
			return TRUE;
	}
	return FALSE;
}