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