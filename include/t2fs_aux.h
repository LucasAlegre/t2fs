#ifndef __T2FS_AUX___
#define __T2FS_AUX___

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apidisk.h"
#include "bitmap2.h"
#include "constants.h"

//*********** TYPES ****************
typedef struct t2fs_superbloco SuperBlock;
typedef struct t2fs_record Record;

typedef struct t2fs_openfile{
	struct t2fs_record record;
	DWORD currentPointer; // Em bytes a partir do inicio do arquivo!
	DWORD entryBlock;  // Bloco com a entrada do arquivo no disco
} OpenFile;


//********* VARIÁVEIS GLOBAIS ********************
SuperBlock superBlock;
OpenFile openFiles[MAX_OPEN_FILES];
OpenFile openDirs[MAX_OPEN_DIR];

int inodeAreaStartSector;
int dataAreaStartBlock;
BOOL initalizedT2fs = FALSE;

char currentPath[MAX_FILE_NAME_SIZE+1];
DWORD currentDirInode;


int readSuperBlock();
void initializeOpenFiles();
int getInode(DWORD inodeNumber, struct t2fs_inode *inode);
int getRecords(DWORD blockNumber, Record *records);
void initializeT2fs();
int getRecordFromEntryBlock(DWORD blockNumber, char *filename, Record *record);
FILE2 getFreeFileHandle();
int getPointers(DWORD blockNumber, DWORD *pointers);

#endif
