#define DEBUG TRUE

#define	SECTOR_SIZE	256 //Bytes

#define TYPEVAL_INVALIDO    0x00
#define TYPEVAL_REGULAR     0x01
#define TYPEVAL_DIRETORIO   0x02

#define	INVALID_PTR	-1

#define BOOL unsigned short int
#define TRUE 1
#define FALSE 0

#define INODE_SIZE 32 //Bytes
#define RECORD_SIZE 64
#define INODE_PER_SECTOR 8
#define RECORD_PER_SECTOR 4
#define PTR_PER_SECTOR 64
#define MAX_OPEN_FILES 10