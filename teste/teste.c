#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "t2fs.h"

int main(){

	char a[1000];
	identify2(a, 100);

	char c[3] = "aa\0";
	int i ;
	for(i = 0; i < 20; i++){
		c[1] += 1;
		int i = create2(c);
		close2(i);
	}
			/*
		char *name = "dir1/dir11/../dir11/file111\0";
		int h = open2(name);
		if (isFileHandleValid(h)){
			printf("%s\n", openFiles[h].record.name);
			printf("%d\n", openFiles[h].record.inodeNumber);
			printf("%d\n", openFiles[h].record.TypeVal);
		}*/
		/*
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
		}*/
			/*Record records[RECORD_PER_BLOCK];
	int i;
	getInodeFromInodeNumber(0, &inode);
	DWORD pointers[POINTERS_PER_BLOCK];
	getPointers(inode.singleIndPtr, pointers);
	getRecordsFromEntryBlock(pointers[0], records);
	for(i = 0; i < RECORD_PER_BLOCK; i++ )
		if(records[i].TypeVal != TYPEVAL_INVALIDO)
			printf("%s\n", records[i].name);

	}*/

	return 0;
}