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
Função: Retorna um DIR handle livre
Saída:	O número do handle livre
        -1 Não há handle livre
-----------------------------------------------------------------------------*/
DIR2 getFreeDirHandle();

/*-----------------------------------------------------------------------------
Função: Retorna PTR_PER_SECTOR*BLOCKSIZE ponteiros no vetor pointers
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int getPointers(DWORD blockNumber, DWORD *pointers);

/*-----------------------------------------------------------------------------
Função: Retorna na variavel record a entrada com o nome dado no bloco de diretório
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int gerRecordFromDir(Inode dirInode, char *filename, Record *recordOut);

/*------------------------TO DO-----------------------------------------------------
Função: Retorna na variavel record a entrada do arquivo com caminho (absoluto ou relativo) em pathname
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int getRecordFromPath(char *pathname, Record *recordOut);

/*------------------------TO DO-----------------------------------------------------
Função: Procura o dirent de numero 'pointer' no diretorio 'inodeNumber'.
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int getRecordFromNumber(DWORD inodeNumber, int pointer, DIRENT2 *dirent);

/*------------------------TO DO-----------------------------------------------------
Função: Procura o inode do diretório pai do arquivo folha (regular ou diretorio) passado em pathname (caminho relativo ou absoluto)
Exemplo: pathname /a/b/c => retorna o inode de b
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int getLastDirInode(char *pathname, Inode *inode);

/*------------------------TO DO-----------------------------------------------------
Função: Verifica se o diretório está vazio (não possui nenhuma entrada válida)
Saída:	Se vazio, true. Se existir alguma entrada, false
-----------------------------------------------------------------------------*/
BOOL isDirEmpty(Inode *dirInode);

/*------------------------TO DO-----------------------------------------------------
Função: Libera todos os blocos de dados do inode
-----------------------------------------------------------------------------*/
void removeAllDataFromInode(Inode *inode);

/*------------------------TO DO-----------------------------------------------------
Função: Procura pela ocorrência do record de mesmo nome do record parâmetro e substitui pelo passado em parâmetro
-----------------------------------------------------------------------------*/
void updateRecord(Inode *inode, Record record);

/*------------------------TO DO-----------------------------------------------------
Função: Inicializa um inode para um novo diretório
-----------------------------------------------------------------------------*/
void initNewDirInode(DWORD inodeNumber);

/*------------------------TO DO-----------------------------------------------------
Função: Acidiona a entrada recebida em record no inode
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int addRecordOnDir(Inode *inode, Record record);

void printError(char *error);

/*----------------------------------------------------------------------------------
Função: Remove .s e ..s de um caminho
-----------------------------------------------------------------------------*/
void fixPath(char* path);

#endif
