#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apidisk.h"
#include "bitmap2.h"
#include "t2fs.h"
#include "constants.h"

typedef struct t2fs_superbloco SuperBlock;
typedef struct t2fs_record Record;

typedef struct t2fs_openfile{
	struct t2fs_record record;
	DWORD currentPointer; // Em bytes a partir do inicio do arquivo!
	DWORD entryBlock;  // Bloco com a entrada do arquivo no disco
} OpenFile;


// Declarar aqui as vaŕiáveis globais
SuperBlock superBlock;
OpenFile openFiles[MAX_OPEN_FILES];

int inodeAreaStartSector;
int dataAreaStartBlock;
BOOL initalizedT2fs = FALSE;

char currentPath[MAX_FILE_NAME_SIZE+1];
DWORD currentDirInode;


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

//************************************* DAQUI PRA BAIXO SÃO AS FUNÇÕES PÚBLICAS *****************************************//

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

	// Ver se ja existe
	// Percorre todas entradas do diretorio atual, ver se ja existe o arquivo com mesmo nome, e acha entrada livre
	// Completa corretamente as estruturas t2fs_record e t2fs_inode pro arquivo
	// setar o inode como ocupado
	// Pointeiro do arquivo no 0

	// ---- ? Começa com 0 ou 1 bloco?
	// ---- ? Abre arquivo tambem?

	return -1;
}


int delete2 (char *filename){
	initializeT2fs();

	// Acha a entrada com o arquivo
	// ??? --- Ver se ta aberto
	// Marca como livre todos os blocos do arquivo
	// Marca o inode como livre
	// Marca a entrada no diretorio como TYPEVAL_INVALIDO
	// ??? -- Arruma o tamanho do diretorio?

	return -1;
}


FILE2 open2 (char *filename){
	initializeT2fs();

	FILE2 freeHandle = getFreeFileHandle(); 
	if(freeHandle == -1) 
		return -1;

	struct t2fs_inode dirInode;
	/// AQUI É O DIRETORIO ATUAL OU TEM QUE ACHAR O DIRETORIO ???
	if(getInode(currentDirInode, &dirInode) == -1) 
		return -1; 

	Record record;
	int i, j;
	// Search on direct pointers
	for(i = 0; i < 2; i++){
		if(dirInode.dataPtr[i] != INVALID_PTR){
			if(getRecordFromEntryBlock(dirInode.dataPtr[i], filename, &record) == 0){
				if(record.TypeVal == TYPEVAL_REGULAR){
					openFiles[freeHandle].record = record;
					openFiles[freeHandle].currentPointer = 0;
					openFiles[freeHandle].entryBlock = dirInode.dataPtr[i];
					return freeHandle;
				}
			}
		}
	}
	// Search on simple indirection
	if(dirInode.singleIndPtr != INVALID_PTR){
		DWORD pointers[PTR_PER_SECTOR*superBlock.blockSize];
		getPointers(dirInode.singleIndPtr, pointers);
		for(i = 0; i < PTR_PER_SECTOR*superBlock.blockSize; i++){
			if(pointers[i] != INVALID_PTR){
				if(getRecordFromEntryBlock(pointers[i], filename, &record) == 0){
					if(record.TypeVal == TYPEVAL_REGULAR){
						openFiles[freeHandle].record = record;
						openFiles[freeHandle].currentPointer = 0;
						openFiles[freeHandle].entryBlock = pointers[i];
						return freeHandle;
					}
				}
			}
		}
	}
	// Search on double indirection
	if(dirInode.doubleIndPtr != INVALID_PTR){
		DWORD doublePointers[PTR_PER_SECTOR*superBlock.blockSize];
		getPointers(dirInode.doubleIndPtr, doublePointers);
		for(i = 0; i < PTR_PER_SECTOR*superBlock.blockSize; i++){
			if(doublePointers[i] != INVALID_PTR){
				DWORD pointers[PTR_PER_SECTOR*superBlock.blockSize];
				getPointers(doublePointers[i], pointers);
				for(j = 0; j < PTR_PER_SECTOR*superBlock.blockSize; j++){
					if(pointers[j] != INVALID_PTR){
						if(getRecordFromEntryBlock(pointers[j], filename, &record) == 0){
							if(record.TypeVal == TYPEVAL_REGULAR){
								openFiles[freeHandle].record = record;
								openFiles[freeHandle].currentPointer = 0;
								openFiles[freeHandle].entryBlock = pointers[j];
								return freeHandle;
							}
						}
					}
				}
			}
		}
	}

	// Ver se tem espaço livre no vetor de OpenFiles
	// ??? --- Procura no diretorio atual ou filename é um path?
	// Coloca no OpenFiles
	// Current pointer começa em 0

	return -1; // File not found
}


int close2 (FILE2 handle){
	initializeT2fs();

	// Libera a entrada no OpenFiles
	// ??? --- Escreve o record no handle no disco ou o disco ta sempre consistente?

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

	// Ver se o arquivo no handle ta aberto
	// Acessar o inode
	// Ve se tem espaço e acha o bloco onde vai escrever
	// (Blocos de indices etc etc)
	// Escreve
	// Atualiza tamanho

	return -1;
}


int truncate2 (FILE2 handle){
	initializeT2fs();

	// Ver se o arquivo no handle ta aberto
	// Acessar o inode
	// Calcula os bytes que vao ser apagados
	// Sai marcando os blocos como livres(cuidar bloco de indices)
	// Atualiza tamanho do arquivo

	return -1;
}

int seek2 (FILE2 handle, DWORD offset){
	initializeT2fs();

	//	O parâmetro "offset" corresponde ao deslocamento, em bytes, contados a partir do início do arquivo.
	//  Se o valor de "offset" for "-1", o current_pointer deverá ser posicionado no byte seguinte ao final do arquivo,
	//	Isso é útil para permitir que novos dados sejam adicionados no final de um arquivo já existente.
	return -1;
}


int mkdir2 (char *pathname){
	initializeT2fs();

	// -- Navega até o último diretorio
	// -- Acha entrada livre
	// -- Cria arquivo
	   // -- Tem que criar duas entradas "./" e "../" !
	   // ??? Entrada . e .. conta como tamanho do diretorio?
	// ?? O disco sempre vem com o / pronto ou tem que criar o / ?
	
	return -1;
}


int rmdir2 (char *pathname){
	initializeT2fs();

    //	São considerados erros quaisquer situações que impeçam a operação.
	//	Isso inclui:
	//		(a) o diretório a ser removido não está vazio;
	//		(b) "pathname" não existente;
	//		(c) algum dos componentes do "pathname" não existe (caminho inválido);
	//		(d) o "pathname" indicado não é um arquivo;

	// Faz parecido com delete

	return -1;
}


int chdir2 (char *pathname){
	initializeT2fs();

	// Navega no pathname
	// 

	return -1;
}


int getcwd2 (char *pathname, int size){
	initializeT2fs();
	return -1;
}

DIR2 opendir2 (char *pathname){
	initializeT2fs();
	return -1;
}

int readdir2 (DIR2 handle, DIRENT2 *dentry){
	initializeT2fs();
	return -1;
}

int closedir2 (DIR2 handle){
	initializeT2fs();
	return -1;
}