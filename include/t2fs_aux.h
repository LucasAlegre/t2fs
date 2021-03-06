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
Função: Inicializa vetor de diretórios abertos openDirs
-----------------------------------------------------------------------------*/
void initializeOpenDirs();

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
int getRecordFromDir(Inode dirInode, char *filename, Record *recordOut);

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
int getRecordFromNumber(DWORD inodeNumber, int pointer, Record *record);

/*------------------------TO DO-----------------------------------------------------
Função: Procura o inode do diretório pai do arquivo folha (regular ou diretorio) passado em pathname (caminho relativo ou absoluto)
Exemplo: pathname /a/b/c => retorna o inode de b
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int getLastDirInode(char *pathname, Inode *inode, int *inodeNumber);

/*-----------------------------------------------------------------------------
Função: Verifica se o diretório está vazio (não possui nenhuma entrada válida)
Saída:	Se vazio, true. Se existir alguma entrada, false

OBS: Ignorar . e ..
-----------------------------------------------------------------------------*/
BOOL isDirEmpty(Inode dirInode);

/*-----------------------------------------------------------------------------
Função: Libera todos os blocos de dados do inode
-----------------------------------------------------------------------------*/
void removeAllDataFromInode(int inodeNumber);

/*-----------------------------------------------------------------------------
Função: Procura pela ocorrência do record de mesmo nome do record parâmetro e substitui pelo passado em parâmetro
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int updateRecord(Inode dirInode, Record recordToChange, BYTE typeVal);

/*-----------------------------------------------------------------------------
Função: Inicializa um inode para um novo diretório
-----------------------------------------------------------------------------*/
int initNewDirInode(int inodeNumber, int inodeNumberPreviousDir);

/*-----------------------------------------------------------------------------
Função: Acidiona a entrada recebida em record no inode
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int addRecordOnDir(Inode *dirInode, Record record);

/*-----------------------------------------------------------------------------
Função: Realiza a escrita do record dado, na entrada de número recordNum do
bloco de entradas dado por blockNum
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int writeRecordOnDir(DWORD blockNum, Record record, int recordNum);

/*
Função: Autoexplicativo :P
*/
void printError(char *error);

/*-----------------------------------------------------------------------------
Saída:	Retorna TRUE se válido, FALSE caso contrário.
-----------------------------------------------------------------------------*/
BOOL isDirHandleValid(DIR2 handle);

/*-----------------------------------------------------------------------------
Saída:	Retorna TRUE se válido, FALSE caso contrário.
-----------------------------------------------------------------------------*/
BOOL isFileHandleValid(FILE2 handle);

/*-----------------------------------------------------------------------------
Função: Inicializa um inode novo para um arquivo, alocando um bloco de dados.
Saída: O número do i-node alocado caso sucesso.
       -1 em caso de erro.
-----------------------------------------------------------------------------*/
int initNewFileInode();

/*-----------------------------------------------------------------------------
Função: Escreve a dword no buffer a partir da posição dada por start.
-----------------------------------------------------------------------------*/
void writeDwordOnBuffer(unsigned char *buffer, int start, DWORD dword);

/*-----------------------------------------------------------------------------
Função: Escreve o inode dado no inode de posiçao inodeNumber no disco.
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int writeInodeOnDisk(Inode inode, int inodeNumber);

/*-----------------------------------------------------------------------------
Função: Inicializa um bloco de entradas de diretório vazias. 
Saída: O número do bloco alocado caso sucesso.
       -1 em caso de erro.
-----------------------------------------------------------------------------*/
int initNewEntryBlock();

/*-----------------------------------------------------------------------------
Função: Inicializa um bloco de ponteiros inválidos. 
Saída: O número do bloco alocado caso sucesso.
       -1 em caso de erro.
-----------------------------------------------------------------------------*/
int initNewPointerBlock();

/*-----------------------------------------------------------------------------
Função: Remove .s e ..s de um caminho
-----------------------------------------------------------------------------*/
void fixPath(char* path);

/*-----------------------------------------------------------------------------
Função: Retorna o nome do arquivo dado um path
        Ex: /a/b/c  ->  c
-----------------------------------------------------------------------------*/
void getFilenameFromPath(char *pathname, char *filename);

/*-----------------------------------------------------------------------------
Função: Retorna TRUE se arquivo aberto, FALSE caso contrário
-----------------------------------------------------------------------------*/
BOOL isFileOpen(int inodeNumber);

/*-----------------------------------------------------------------------------
Função: Retorna TRUE se diretório aberto, FALSE caso contrário
-----------------------------------------------------------------------------*/
BOOL isDirOpen(int inodeNumber);

/*-----------------------------------------------------------------------------
Função: Atualiza em disco o inode de um diretório
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int updateDirInode(Inode dirInode);

/*-----------------------------------------------------------------------------
Função: Escreve em disco um ponteiro no bloco dado na posição index
Saída:	Se a operação foi realizada com sucesso, a função retorna "0" (zero).
	Em caso de erro, será retornado um valor diferente de zero.
-----------------------------------------------------------------------------*/
int writePointerOnBlock(DWORD blockNum, DWORD pointer, int index);

/*-----------------------------------------------------------------------------
Função: Lê a quantidade de bytes passada por parâmetro do arquivo correspondente ao i-node do parâmetro
Saída:	os bytes lidos estarão no array buffer
-----------------------------------------------------------------------------*/
void readBytesFromFile(DWORD currentPointer, Inode fileInode, int numBytes, char *buffer);

/*-----------------------------------------------------------------------------
Função: Lê maxBytes a partir de pointer no bloco blockNumber e passa os bytes lidos para buffer
Saída:	número de bytes lidos
-----------------------------------------------------------------------------*/
int readBlock(int blockNumber, int pointer, int maxBytes, char *buffer);

/*-----------------------------------------------------------------------------
Função: Libera maxBlocks do arquivo de fileInode a partir do startBlock
Saída:	os bytes lidos estarão no array buffer
-----------------------------------------------------------------------------*/
void freeBlocks(Inode fileInode, int inodeNumber, int startBlock, int maxBlocks);

/*-----------------------------------------------------------------------------
Função: Escreve os bytes em buffer no arquivo de inodeNumber a partir de currentPointer e coloca o número de bytes escritos em numBytes
Saída:	0 se deu tudo certo, -1 caso contrário
-----------------------------------------------------------------------------*/
int writeBytesOnFile(DWORD currentPointer, int inodeNumber, char *buffer, int size, int *numBytes);

/*-----------------------------------------------------------------------------
Função: Escreve os bytes de buffer no bloco blockNumber
-----------------------------------------------------------------------------*/
void writeBlock(int blockNumber, char *buffer);
#endif
