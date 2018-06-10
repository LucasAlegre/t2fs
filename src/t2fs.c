#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apidisk.h"
#include "bitmap2.h"
#include "t2fs.h"
#include "t2fs_aux.h"
#include "constants.h"


int identify2 (char *name, int size){
	initializeT2fs();

	char *group = "Lucas Nunes Alegre 00274693\nAline Weber 00274720\nLucas Sonntag Hagen 00274698\0";
	if(size < strlen(group)){
		printf("Size given is not sufficient to copy the whole string!\n");
		return -1;
	}
	strncpy(name, group, strlen(group)+1);

	return 0;
}


FILE2 create2 (char *filename){
	initializeT2fs();

	Inode dirInode;
	Record record;
	int number;
	char filenameSohArquivo[MAX_FILE_NAME_SIZE+1];

	if(getLastDirInode(filename, &dirInode, &number) != 0)
		return -1;

	getFilenameFromPath(filename, filenameSohArquivo);
	if(getRecordFromDir(dirInode, filenameSohArquivo, &record) == 0)
		return -1; //arquivo ja existe

	strcpy(record.name, filenameSohArquivo);
	record.TypeVal = TYPEVAL_REGULAR;
	int inodeNum = initNewFileInode();
	if(inodeNum == -1)
		return -1;

    record.inodeNumber = inodeNum;

	if(addRecordOnDir(&dirInode, record) != 0){
		removeAllDataFromInode(inodeNum);
		setBitmap2(BITMAP_INODE, inodeNum, 0);
		return -1;
	}

	return open2(filename);
}


int delete2 (char *filename){
	initializeT2fs();

	Inode dirInode;
	Record record;
	int number;
	char filenameSohArquivo[MAX_FILE_NAME_SIZE+1];

	if(getLastDirInode(filename, &dirInode, &number) != 0)
		return -1;

	getFilenameFromPath(filename, filenameSohArquivo);
	if(getRecordFromDir(dirInode, filenameSohArquivo, &record) != 0)
		return -1;

	if(!isFileOpen(record.inodeNumber) && record.TypeVal == TYPEVAL_REGULAR){	 
		record.TypeVal = TYPEVAL_INVALIDO;
		removeAllDataFromInode(record.inodeNumber);
		setBitmap2 (BITMAP_INODE, record.inodeNumber, 0);
	    record.inodeNumber = INVALID_PTR;
	    if(updateRecord(dirInode, record, TYPEVAL_REGULAR) != 0)
	    	return -1;

	    return 0;
	}
	
	return -1;
}


FILE2 open2 (char *filename){
	initializeT2fs();

	FILE2 freeHandle = getFreeFileHandle(); 
	if(freeHandle == -1) 
		return -1;  // OpenFiles is full

	Record record;
	if(getRecordFromPath(filename, &record) != 0){
		return -1;
	}

	if(record.TypeVal == TYPEVAL_REGULAR){
		openFiles[freeHandle].record = record;
		openFiles[freeHandle].currentPointer = 0;
		return freeHandle;
	}
	return -1;
}


int close2 (FILE2 handle){
	initializeT2fs();

	if(isFileHandleValid(handle)){
	 	openFiles[handle].record.TypeVal = TYPEVAL_INVALIDO;
		openFiles[handle].record.inodeNumber = INVALID_PTR;
        return 0;
    }

	return -1;
}


int read2 (FILE2 handle, char *buffer, int size){
	initializeT2fs();


	OpenFile file;
	Inode fileInode;
	int numBytes = 0;

	if(isFileHandleValid(handle)){
		file = openFiles[handle];
		if(getInodeFromInodeNumber(file.record.inodeNumber, &fileInode) == 0){
			if(file.currentPointer < fileInode.bytesFileSize){ //se não tá no fim do arquivo
				if(file.currentPointer+size <= fileInode.bytesFileSize){ // caso seja possível ler todos os size bytes
					readBytesFromFile(file.currentPointer, fileInode, size, buffer);
					file.currentPointer = file.currentPointer+size;
					numBytes = size;
				}
				else{ //senão lê só o que for possível
					numBytes = fileInode.bytesFileSize - file.currentPointer;
					readBytesFromFile(file.currentPointer, fileInode, numBytes, buffer);
					file.currentPointer = fileInode.bytesFileSize;
				}
				openFiles[handle] = file; //atualiza openFiles (currentPointer)
			}
			return numBytes;
		}
	}

	return -1;
}


int write2 (FILE2 handle, char *buffer, int size){
	initializeT2fs();

	OpenFile file;
	int numBytes;

	if(isFileHandleValid(handle)){
		file = openFiles[handle];

		if(writeBytesOnFile(file.currentPointer, file.record.inodeNumber, buffer, size, &numBytes) == 0){
			file.currentPointer += numBytes;
			openFiles[handle] = file;
			return numBytes;
		}	
		return -1;
	}
	return -1;
}


int truncate2 (FILE2 handle){
	initializeT2fs();

	OpenFile file;
	Inode fileInode;

	if(isFileHandleValid(handle)){
		file = openFiles[handle];
		int currentBlock;

		if(getInodeFromInodeNumber(file.record.inodeNumber, &fileInode) == 0){
		
			if(file.currentPointer < fileInode.bytesFileSize){
				currentBlock = file.currentPointer/(BLOCK_SIZE*SECTOR_SIZE);


				if(fileInode.blocksFileSize > currentBlock+1){
					if(file.currentPointer-currentBlock*BLOCK_SIZE*SECTOR_SIZE == 0){
						freeBlocks(fileInode, file.record.inodeNumber, currentBlock+1, fileInode.blocksFileSize);
					}
					else{
						freeBlocks(fileInode, file.record.inodeNumber, currentBlock+2, fileInode.blocksFileSize);
					}
				}
				fileInode.bytesFileSize = file.currentPointer;
				fileInode.blocksFileSize = file.currentPointer/(BLOCK_SIZE*SECTOR_SIZE) +1;	
				writeInodeOnDisk(fileInode, file.record.inodeNumber);
			}	

			return 0;
		}
	}

	return -1;
}

int seek2 (FILE2 handle, DWORD offset){
	initializeT2fs();

	OpenFile file;
	Inode fileInode;
	file = openFiles[handle];

	if(file.record.TypeVal == TYPEVAL_REGULAR){
		if(offset != -1)
			file.currentPointer = offset;
		else{
			if(getInodeFromInodeNumber(file.record.inodeNumber, &fileInode) == 0)
				file.currentPointer = fileInode.bytesFileSize;
		}
		openFiles[handle] = file;
		return 0;
	}

	return -1;
}


int mkdir2 (char *pathname){
	initializeT2fs();

	Inode previousDirInode;
	Record record;
	int previousDirInodeNumber;
	char dirName[MAX_FILE_NAME_SIZE+1];

	if(getLastDirInode(pathname, &previousDirInode, &previousDirInodeNumber) == 0){
		getFilenameFromPath(pathname, dirName);
	   	if(getRecordFromDir(previousDirInode, dirName, &record) != 0){ //se esse dir não existe
		    strcpy(record.name, dirName);
		    record.TypeVal = TYPEVAL_DIRETORIO;
		    int inodeNum = searchBitmap2(BITMAP_INODE, 0);
			if(inodeNum != -1){
	    		record.inodeNumber = inodeNum;
			}
		    else{
		    	return -1;
		    }
		    if(initNewDirInode(inodeNum, previousDirInodeNumber) == 0){
		    	if(addRecordOnDir(&previousDirInode, record) == 0){
		    		return 0;
		    	}
		    	else{
		    		removeAllDataFromInode(inodeNum);
					setBitmap2(BITMAP_INODE, inodeNum, 0);
					return -1;
		    	}
		    }
		}
		return -1;
	}
	return -1;
}


int rmdir2 (char *pathname){
	initializeT2fs();

	Inode dirInode, previousDirInode;
	Record record;
	int number;
	char dirName[MAX_FILE_NAME_SIZE+1];

	if(getLastDirInode(pathname, &previousDirInode, &number) == 0){
		getFilenameFromPath(pathname, dirName);
		if(getRecordFromDir(previousDirInode, dirName, &record) == 0){
			getInodeFromInodeNumber(record.inodeNumber, &dirInode);
			if(record.TypeVal == TYPEVAL_DIRETORIO && isDirEmpty(dirInode) && !isDirOpen(record.inodeNumber)){ 
	     		record.TypeVal = TYPEVAL_INVALIDO;
	     		removeAllDataFromInode(record.inodeNumber);
	     		setBitmap2(BITMAP_INODE, record.inodeNumber, 0);
	     		record.inodeNumber = INVALID_PTR;
	     		if(updateRecord(previousDirInode, record, TYPEVAL_DIRETORIO) == 0)
					return 0;
			}
		}
	}

	return -1;
}


int chdir2 (char *pathname){
	initializeT2fs();

	Record record;

	if(getRecordFromPath(pathname, &record) != 0) {
		printError("Arquivo não encontrado!");
		return -1;
	}

	if(record.TypeVal != TYPEVAL_DIRETORIO) {
		printError("Arquivo não é diretório ou inválido!");
		return -2;
	}

	currentDirInode = record.inodeNumber;

	if(*pathname == '/') {
		strncpy(currentPath, pathname, MAX_FILE_NAME_SIZE+1);
		fixPath(currentPath);
	} else {
		char path[MAX_FILE_NAME_SIZE * 2 + 2] = {0};
		strncpy(path, currentPath, MAX_FILE_NAME_SIZE+1);

		strcat(path, pathname);

		fixPath(path);
		strncpy(currentPath, path, MAX_FILE_NAME_SIZE+1);
	}

	return 0;
}


int getcwd2 (char *pathname, int size){
	initializeT2fs();

	strncpy(pathname, currentPath, size);
	return 0;
}

DIR2 opendir2 (char *pathname){
	initializeT2fs();

	DIR2 freeHandle = getFreeDirHandle(); 
	if(freeHandle == -1) 
		return -1;  // OpenFiles is full

	Record record;
	if(getRecordFromPath(pathname, &record) != 0){
		return -1;
	} 

	if(record.TypeVal != TYPEVAL_DIRETORIO){
		return -1;
	}

	openDirs[freeHandle].record = record;
	openDirs[freeHandle].currentPointer = 0;
	return freeHandle;
}

int readdir2 (DIR2 handle, DIRENT2 *dentry){
	initializeT2fs();

	Record record;
	Inode inode;
	Inode recordInode;
	int i;

	if(!isDirHandleValid(handle)) {
		printError("Handle Invalido");
		return -1;
	}

	if(getInodeFromInodeNumber(openDirs[handle].record.inodeNumber, &inode) != 0 ||
		getRecordFromNumber(openDirs[handle].record.inodeNumber, openDirs[handle].currentPointer, &record) != 0) {
		return -2;
	}

	openDirs[handle].currentPointer++;

	if(record.TypeVal == TYPEVAL_INVALIDO) {
		return readdir2(handle, dentry);
	}

	for(i = 0; i <= MAX_FILE_NAME_SIZE; i++)
		dentry->name[i] = '\0';

	getInodeFromInodeNumber(record.inodeNumber, &recordInode);
	strncpy(dentry->name, record.name, MAX_FILE_NAME_SIZE);
	dentry->fileType = record.TypeVal;
	dentry->fileSize = recordInode.bytesFileSize;

	return 0;
}

int closedir2 (DIR2 handle){
	initializeT2fs();

	if(!isDirHandleValid(handle)) {
		return -1;
	}

	openDirs[handle].record.TypeVal = TYPEVAL_INVALIDO;
	return 0;
}