#ifndef __T2FS_AUX___
#define __T2FS_AUX___

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "apidisk.h"
#include "bitmap2.h"
#include "constants.h"
#include "t2fs.h"

//************************ TYPES ************************
typedef struct t2fs_superbloco SuperBlock;
typedef struct t2fs_record Record;
typedef struct t2fs_inode Inode;

typedef struct t2fs_openfile{
	struct t2fs_record record;
	DWORD currentPointer; // Em bytes a partir do inicio do arquivo!
} OpenFile;


//************************ VARIÁVEIS GLOBAIS ************************
SuperBlock superBlock;
OpenFile openFiles[MAX_OPEN_FILES];
OpenFile openDirs[MAX_OPEN_DIR];

int inodeAreaStartSector;
int dataAreaStartBlock;

char currentPath[MAX_FILE_NAME_SIZE+1];
DWORD currentDirInode;


// ************************ FUNÇÕES ************************
/*-----------------------------------------------------------------------------
Função: Inicialização da Biblioteca
-----------------------------------------------------------------------------*/
void initializeT2fs();

/*-----------------------------------------------------------------------------
Função: Lê o super bloco do disco no setor 0 e salva na varíavel superBlock
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int readSuperBlock();

/*-----------------------------------------------------------------------------
Função: Inicializa vetor de arquivos abertos openFiles
-----------------------------------------------------------------------------*/
void initializeOpenFiles();

/*-----------------------------------------------------------------------------
Função: Retorna a estrutura inode do disco associado ao inodeNumber
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int getInodeFromInodeNumber(DWORD inodeNumber, struct t2fs_inode *inode);

/*-----------------------------------------------------------------------------
Função: Retorna as RECORD_PER_SECTOR*BLOCKSIZE entradas de um bloco de diretório
no vetor records
Entra:  Número do bloco a ser lido, vetor a ser preenchido
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int getRecordsFromEntryBlock(DWORD blockNumber, Record *records);

/*-----------------------------------------------------------------------------
Função: Retorna na variavel record a entrada com o nome dado no bloco de diretório
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int getRecordFromEntryBlock(DWORD blockNumber, char *filename, Record *record);

/*-----------------------------------------------------------------------------
Função: Retorna um handle livre
Saída:	O número do handle livre
        -1 Não há handle livre
-----------------------------------------------------------------------------*/
FILE2 getFreeFileHandle();

/*-----------------------------------------------------------------------------
Função: Retorna PTR_PER_SECTOR*BLOCKSIZE ponteiros no vetor pointers
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int getPointers(DWORD blockNumber, DWORD *pointers);

int gerRecordFromDir(Inode dirInode, char *filename, Record *recordOut);

int getRecordFromPath(char *filename, Record *recordOut);

/*
   Exemple: /a/b/c
   Return b inode
*/
int getLastDirInode(char *pathname, Inode *inode);

BOOL isDirEmpty(Inode *dirInode);



#endif
