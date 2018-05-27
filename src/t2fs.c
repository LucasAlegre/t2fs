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

	// Inode dirInode, fileInode;
	// Record record;
	//if getLastDirInode(filenameCompleto, &dirInode) == 0
	//    if(getRecordFromDir(dirInode, filenameSohArquivo, &record) == 0)
	//          return -1; //arquivo ja existe
	//	 else
	//		strcpy(record.name, filenameSohArquivo);
	//      record.Typeval = TYPEVAL_REGULAR;
	//      int inodeNum = searchBitmap2(BITMAP_INODE, 0));
	//		if(inodeNum != -1)
    //			record.inodeNumber = inodeNum;
	//      else
	//         return -1 
	//		initNewFileInode(inodeNum)  --> Deve marcar como ocupado no bitmap, inicializar um bloco de dados vazio
	//      writeRecordOnDir(dirInode, record)
	//      return open2(filenameCompleto)

	return -1;
}


int delete2 (char *filename){
	initializeT2fs();

	// Inode dirInode, fileInode;
	// Record record;
	//if getLastDirInode(filenameCompleto, &dirInode) == 0
	//    if(getRecordFromDir(dirInode, filenameSohArquivo, &record) == 0)
	//		  if(!isOpen(record.inode) && record.Typeval == TYPEVAL_REGULAR)
	  	//	    
		//      record.Typeval = TYPEVAL_INVALIDO;
		//      removeAllDataFromInode(record.inode);
		//      setBitmap2 (BITMAP_INODE, record.inode, 0);
	    //      record.inodeNumber = INVALID_PTR:
	    //      updateRecord(dirInode, record) --> procura pelo nome

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

	// if( isFileHandleValid(handle))
	// 		openFiles[handle].record.Typeval = TYPEVAL_INVALIDO;
	//		openFiles[handle].record.inodeNumber = INVALID_PTR;
	//      return 0;

	return -1;
}


int read2 (FILE2 handle, char *buffer, int size){
	initializeT2fs();

	// Ver se o arquivo no handle ta aberto
	// Acessar o inode
	// ??? --- Ver se tem size bytes pra ler
	// Ler size bytes a partir do current pointer
	// Atualizar o current pointer

	return -1;
}


int write2 (FILE2 handle, char *buffer, int size){
	initializeT2fs();
/*
	OpenFile file;
	Inode fileInode;
	file = openFiles[handle];

	if(file.record.TypeVal == TYPEVAL_REGULAR){
		fileInode = getInodeFromInodeNumber(record.inodeNumber);

		

		return 0;
	}
*/
	// Ve se tem espaço e acha o bloco onde vai escrever
	// (Blocos de indices etc etc)
	// Escreve
	// Atualiza tamanho

	return -1;
}


int truncate2 (FILE2 handle){
	initializeT2fs();
/*
	OpenFile file;
	Inode fileInode;
	file = openFiles[handle];

	if(file.record.TypeVal == TYPEVAL_REGULAR){
		fileInode = getInodeFromInodeNumber(record.inodeNumber);
		
		if(file.currentPointer < fileInode.bytesFileSize + 1)
		
		

		return 0;
	}
*/

	// Remove do arquivo todos os bytes a partir da posição atual do contador de posição (CP)
	// Todos os bytes a partir da posição CP (inclusive) serão removidos do arquivo.
	// Após a operação, o arquivo deverá contar com CP bytes e o ponteiro estará no final do arquivo
	
	// Calcula os bytes que vao ser apagados
	// Sai marcando os blocos como livres(cuidar bloco de indices)
	// Atualiza tamanho do arquivo

	return -1;
}

int seek2 (FILE2 handle, DWORD offset){
	initializeT2fs();

/*
	OpenFile file;
	Inode fileInode;
	file = openFiles[handle];

	if(file.record.TypeVal == TYPEVAL_REGULAR){
		if(offset != -1)
			file.currentPointer = offset;
		else{
			fileInode = getInodeFromInodeNumber(record.inodeNumber);
			file.currentPointer = fileInode.bytesFileSize + 1; (not sure)
		}
		openFiles[handle] = file;
		return 0;
	}
*/
	return -1;
}


int mkdir2 (char *pathname){
	initializeT2fs();

/*
	Inode dirInode, previousDirInode;
	Record record;

	if(getLastDirInode(filenameCompleto, &dirInode) == 0){
	   	if(getRecordFromDir(previousDirInode, dirName, &record) != 0){ //se esse dir não existe
		    strcpy(record.name, dirName);
		    record.Typeval = TYPEVAL_DIRETORIO;
		    int inodeNum = searchBitmap2(BITMAP_INODE, 0));
			if(inodeNum != -1)
	    		record.inodeNumber = inodeNum;
		    else
		    	return -1;
			initNewDirInode(inodeNum);
		    if(addRecordOnDir(&dirInode, record) == 0)
		    	return 0;
		}
	}
*/
	return -1;
}


int rmdir2 (char *pathname){
	initializeT2fs();

/*
	Inode dirInode, previousDirInode;
	Record record;
	if(getLastDirInode(pathname, &previousDirInode) == 0){
		if(getRecordFromDir(previousDirInode, dirName, &record) == 0){
			getInodeFromInodeNumber(record.inode, &dirInode);
			if(record.Typeval == TYPEVAL_DIRETORIO && isDirEmpty(dirInode)){ 
	     		record.Typeval = TYPEVAL_INVALIDO;
	     		removeAllDataFromInode(record.inode);
	     		setBitmap2 (BITMAP_INODE, record.inode, 0);
	     		record.inodeNumber = INVALID_PTR:
	     		updateRecord(previousDirInode, record) --> procura pelo nome
				return 0;
			}
		}
	}
*/
	return -1;
}


int chdir2 (char *pathname){
	initializeT2fs();

	Record record;

	if(getRecordFromPath(pathname) != 0) {
		printError("Arquivo não encontrado!");
		return -1;
	}

	if(record.TypeVal != TYPEVAL_DIRETORIO) {
		printError("Arquivo não é diretório ou inválido!");
		return -2;
	}

	currentDirInode = record.inodeNumber;
	strncpy(currentPath, pathname, MAX_FILE_NAME_SIZE+1); // TODO: Fix path if used relative path

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
	if(getRecordFromPath(filename, &record) != 0){
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

	DIRENT2 ent;

	if(openDirs[handle].TypeVal != TYPEVAL_DIRETORIO) {
		printError("Handle Invalido");
		return -1;
	}

	if(getRecordFromNumber(openDirs[handle].record.inodeNumber, openDirs[handle].currentPointer, &ent) != 0) {
		return -2;
	}

	openDirs[handle].currentPointer++;
	*dentry = ent;
	return 0;
}

int closedir2 (DIR2 handle){
	initializeT2fs();

	if(openDirs[handle].TypeVal != TYPEVAL_DIRETORIO) {
		return -1;
	}

	openDirs[handle].TypeVal = TYPEVAL_INVALIDO;
	return 0;
}