//***************************************//
//                                       //
//  Author : Abhijit Kailas Gadhave      //
//  Date : 24/11/2021                    //
//                                       //
//***************************************//

#include<stdio.h>
#include<fcntl.h>
#include<stdlib.h>
#include<unistd.h>
//#include<io.h>
//#include<iostream>
#include<string.h>

#define MAXINODE 50

#define READ 1
#define WRITE 2

#define MAXFILESIZE 1024

#define REGULAR 1
#define SPECIAL 2

#define START 0
#define CURRENT 1
#define END 2

typedef struct superblock
{
	int TotalInodes;
	int FreeInodes;
}SUPERBLOCK,*PSUPERBLOCK;

typedef struct inode
{
	char FileName[50];
	int InodeNumber;
	int FileSize;
	int FileActualSize;
	int FileType;
	char *Buffer;
	int LinkCount;
	int ReferenceCount;
	int Permission; // 1   23
	struct inode *next;
}INODE,*PINODE,**PPINODE;

typedef struct filetable
{
	int readoffset;
	int writeoffset;
	int count;
	int mode;  // 1  2  3 
	PINODE ptrinode;
}FILETABLE,*PFILETABLE;

typedef struct ufdt
{
	PFILETABLE ptrfiletable;
}UFDT;