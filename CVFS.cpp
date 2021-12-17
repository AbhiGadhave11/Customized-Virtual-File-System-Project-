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

UFDT UFDTArr[MAXINODE];
SUPERBLOCK SUPERBLOCKobj;
PINODE head = NULL;
 

///////////////////////////////////////////////
// Name of the function : InitaliseSuperBlock//      
// Input parameters : Nothing                //
// return value : void                       //
// Description of the function : No          // 
// Use : It is used to initalise super block //
//      i.e. store the information about     //
//      blocks and inodes                    //
///////////////////////////////////////////////

void InitaliseSuperBlock()
{
	int i = 0;
	while(i < MAXINODE)
	{
		UFDTArr[i].ptrfiletable = NULL;
		i++;
	}
	SUPERBLOCKobj.TotalInodes = MAXINODE;
	SUPERBLOCKobj.FreeInodes = MAXINODE;
}



///////////////////////////////////////////////
// Name of the function : CreateDILB         //
// Input parameters : Nothing                //
// return value : void                       //
// Description of the function : No          // 
// Use : It is used to create Disk iNode List//
//      Block in that block all inode objects//
//      are present                          //
///////////////////////////////////////////////

void CreateDILB()
{
	int i = 1;
	PINODE newn = NULL;
	PINODE temp = head;

	while(i<= MAXINODE)
	{
		newn = (PINODE)malloc(sizeof(INODE));

        //newn->FileName = NULL;
		newn->LinkCount = 0;
		newn->ReferenceCount = 0;
		newn->FileType = 0;
		newn->FileSize = 0;

		newn->Buffer = NULL;
		newn->next = NULL;

		newn->InodeNumber = i;

		if(temp == NULL)
		{
			head =newn;
			temp = head;
		}
		else
		{
			temp->next = newn;
			temp = temp->next;
		}
		i++;
	}
	printf("DILB Created Successfully\n");
}



///////////////////////////////////////////////
// Name of the function : Get_Inode          //
// Input parameters : character pointer      //
// return value : structure Inode pointer    //
// Description of the function : No          // 
// Use : It gives a remaining one of the     //
//       node on hard disk to our file       //
///////////////////////////////////////////////

PINODE Get_Inode(char *name)
{
	PINODE temp = head;
	int i = 0;

	if(name == NULL)
	{
		return NULL;
	}
	while(temp!=NULL)
	{
		if(strcmp(name,temp->FileName) == 0)
		{
			break;
		}
		temp = temp->next;
	}
	return temp;
}


///////////////////////////////////////////////
// Name of the function : CreateFile         //
// Input parameters : character pointer and  //
//                    Integer                //
// return value : Integer                    //
// Description of the function : No          // 
// Use : It is used to create one of the new //
//      File(Directory)                      //
///////////////////////////////////////////////

int CreateFile(char *name,int Permission)
{
	int i = 0;
	PINODE temp = head;
	PINODE tRet = NULL;
	if((name == NULL)||(Permission == 0)||(Permission > 3))
	{
		return -1;
	}
	if(SUPERBLOCKobj.FreeInodes == 0)
	{
		return -2;
	}
	(SUPERBLOCKobj.FreeInodes)--;
	tRet = Get_Inode(name);
	if(tRet != NULL)
	{
		return -3;
	}
	while(temp!=NULL)
	{
		if(temp->FileType == 0)
		{
			break;
		}
		temp = temp->next;
	}
	while(i<50)
	{
		if(UFDTArr[i].ptrfiletable == NULL)
		{
			break;
		}
		i++;
	}

	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));

	UFDTArr[i].ptrfiletable->count = 1;
	UFDTArr[i].ptrfiletable->mode = Permission;
	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;

	UFDTArr[i].ptrfiletable->ptrinode = temp;

	strcpy(UFDTArr[i].ptrfiletable->ptrinode->FileName,name);
	UFDTArr[i].ptrfiletable->ptrinode->FileType = REGULAR;
	UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount = 1;
	UFDTArr[i].ptrfiletable->ptrinode->LinkCount = 1;
	UFDTArr[i].ptrfiletable->ptrinode->FileSize = MAXFILESIZE;
	UFDTArr[i].ptrfiletable->ptrinode->FileActualSize = 0;
	UFDTArr[i].ptrfiletable->ptrinode->Permission = Permission;
	UFDTArr[i].ptrfiletable->ptrinode->Buffer = (char *)malloc(MAXFILESIZE); 
    
    memset(UFDTArr[i].ptrfiletable->ptrinode->Buffer,0,1024);
    return i;
}



///////////////////////////////////////////////
// Name of the function : OpenFile           //
// Input parameters : character pointer and  //
//                    One Integer            //
// return value : Integer                    //
// Description of the function : No          // 
// Use : It is used to open the file which   //
//       are created by user.                 //
///////////////////////////////////////////////

int OpenFile(char *name,int mode)
{
	int i=0;
	PINODE temp = NULL;

	if((name == NULL) || (mode <= 0))
	{
		return -1;
	}
	temp = Get_Inode(name);
	if(temp == NULL)
	{
		return -2;
	} 
	if((temp->Permission) < mode)
	{
		return -3;
	}
	while(i < 50)
	{
		if(UFDTArr[i].ptrfiletable == NULL)
		{
			break;
		}
		i++;
	}
	UFDTArr[i].ptrfiletable = (PFILETABLE)malloc(sizeof(FILETABLE));
	if(UFDTArr[i].ptrfiletable == NULL)
	{
		return -1;
	}
	UFDTArr[i].ptrfiletable->count = 1;
	UFDTArr[i].ptrfiletable->mode = mode;
	if(mode == READ + WRITE)
	{
		UFDTArr[i].ptrfiletable->readoffset = 0;
		UFDTArr[i].ptrfiletable->writeoffset = 0;
	}
	else if(mode == READ)
	{
		UFDTArr[i].ptrfiletable->readoffset = 0;
	}
	else if(mode == WRITE)
	{
		UFDTArr[i].ptrfiletable->writeoffset = 0;
	}
	UFDTArr[i].ptrfiletable->ptrinode = temp;
	(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)++;
	return i;
}



///////////////////////////////////////////////
// Name of the function : WriteFile          //
// Input parameters : character pointer and  //
//                    Two Integers           //
// return value : Integer                    //
// Description of the function : No          // 
//Use : It's used  write the contents to file//                                  
///////////////////////////////////////////////

int WriteFile(int fd,char *arr,int isize)
{
	
	if(UFDTArr[fd].ptrfiletable->mode!=WRITE && UFDTArr[fd].ptrfiletable->mode != READ + WRITE)
	{
		return -1;
	}

	if(UFDTArr[fd].ptrfiletable->ptrinode->Permission!= WRITE && UFDTArr[fd].ptrfiletable->ptrinode->Permission	!=READ + WRITE)
	{
		return -1;
	}

	if((UFDTArr[fd].ptrfiletable->writeoffset) == MAXFILESIZE)
	{
		return -2;
	}

	if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
	{
		return -3;
	}

	    strncpy((UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->writeoffset),arr,isize);
	    
	    (UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + isize;

		(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;

	return isize;
}


///////////////////////////////////////////////
// Name of the function : ReadFile           //
// Input parameters : character pointer and  //
//                    two Integers           //
// return value : Integer                    //
// Description of the function : No          // 
// Use : Read the contents of file which are //
//      which are created by user            //
///////////////////////////////////////////////

int ReadFile(int fd,char *arr,int isize)
{
	int read_size = 0;

	if(UFDTArr[fd].ptrfiletable == NULL)
	{
		return -1;
	}

	if(UFDTArr[fd].ptrfiletable->mode!=READ && UFDTArr[fd].ptrfiletable->mode != READ + WRITE)
	{
		return -2;
	}

	if((UFDTArr[fd].ptrfiletable->ptrinode->Permission!= READ) && (UFDTArr[fd].ptrfiletable->ptrinode->Permission !=READ + WRITE))
	{
		return -2;
	}

	if(UFDTArr[fd].ptrfiletable->readoffset == UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
	{
		return -3;
	}

	if(UFDTArr[fd].ptrfiletable->ptrinode->FileType != REGULAR)
	{
		return -4;
	}

	read_size = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) - (UFDTArr[fd].ptrfiletable->readoffset);

	if(read_size < isize)
	{
		strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->readoffset),read_size);
	    
	    UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + read_size;
	}
	else
	{
		strncpy(arr,(UFDTArr[fd].ptrfiletable->ptrinode->Buffer)+(UFDTArr[fd].ptrfiletable->readoffset),isize);
	    
	    UFDTArr[fd].ptrfiletable->readoffset = UFDTArr[fd].ptrfiletable->readoffset + isize;
	}

	return isize;
}



///////////////////////////////////////////////
// Name of the function : man                //
// Input parameters : character pointer      //
// return value : void                       //
// Description of the function : No          // 
//Use :                                      //
///////////////////////////////////////////////

void man(char *name)
{
	if(name == NULL)
	{
		return;
	}
	if(strcmp(name,"create") == 0)
	{
		printf("Description : Used to create new regular File\n");
		printf("Usage : create File_name Permission\n");
	}
	else if(strcmp(name,"read") == 0)
	{
		printf("Description : Used to read data from regular File\n");
		printf("Usage : read File_name No_of_bytes_To_Read\n");
	}
	else if(strcmp(name,"write") == 0)
	{
		printf("Description : Used to write into regular File\n");
		printf("Usage : write File_name\nAfter this enter the data that we want to write\n");
	}
	else if(strcmp(name,"ls") == 0)
	{
		printf("Description : Used to list all information of Files\n");
		printf("Usage : ls\n");
	}
	else if(strcmp(name,"stat") == 0)
	{
		printf("Description : Used to Display information of File\n");
		printf("Usage : stat file_name\n");
	}
	else if(strcmp(name,"fstat") == 0)
	{
		printf("Description : Used to Display information of File\n");
		printf("Usage : fstat file_Descriptor\n");
	}
	else if(strcmp(name,"truncate") == 0)
	{
		printf("Description : Used to remove data from regular File\n");
		printf("Usage : truncate File_Name\n");
	}
	else if(strcmp(name,"open") == 0)
	{
		printf("Description : Used to open existing File\n");
		printf("Usage : open File_name mode\n");
	}
	else if(strcmp(name,"close") == 0)
	{
		printf("Description : Used to close opened File\n");
		printf("Usage : close File_name\n");
	}
	else if(strcmp(name,"closeall") == 0)
	{
		printf("Description : Used to close all opened files File\n");
		printf("Usage : closeall\n");
	}
	else if(strcmp(name,"lseek") == 0)
	{
		printf("Description : Used to change File offset\n");
		printf("Usage : lseek File_name ChangeInOffset StartPoint\n");
	}
	else if(strcmp(name,"rm") == 0)
	{
		printf("Description : Used to delete the File\n");
		printf("Usage : rm File_Name\n");
	}
	else
	{
		printf("No Manual Entry Available\n");
	}

}


///////////////////////////////////////////////
// Name of the function : DisplayHelp        //
// Input parameters : Nothing                //
// return value : void                       //
// Description of the function : No          // 
//Use : It Displays(help) the information    //
//      of used commands                     //
///////////////////////////////////////////////

void DisplayHelp()
{
	printf("ls : To List Out all files\n");
	printf("clear : To clear console\n");
	printf("open : To open the file\n");
	printf("close : To close the file\n");
	printf("closeall : To close all opened files\n");
	printf("read : To Read the contents from file\n");
	printf("write : To Write contents into file\n");
	printf("exit : To Terminate file system\n");
	printf("stat : To Display information of file using name\n");
	printf("fstat : To Display information of file using file descriptor\n");
	printf("truncate : To remove all data from file\n");
	printf("rm : To Delete the file\n");
	
}

///////////////////////////////////////////////
// Name of the function : GetFDFromName      //
// Input parameters : character pointer      //
// return value : Integer                    //
// Description of the function : No          // 
//Use :                                      //
///////////////////////////////////////////////


int GetFDFromName(char *name)
{
	int i=0;

	while(i<50)
	{
		if(UFDTArr[i].ptrfiletable != NULL)
		{
			if(strcmp((UFDTArr[i].ptrfiletable->ptrinode->FileName),name) == 0)
			{
				break;
			}
		}
		i++;
	}
	if(i==50)
	{
		return -1;
	}
	else
	{
		return i;
	}
}



///////////////////////////////////////////////
// Name of the function : rm_file            //
// Input parameters : character pointer      //
// return value : Integer                    //
// Description of the function : No          // 
// Use : These function is used to delete the//
//        file                               //
///////////////////////////////////////////////

int rm_file(char *name)
{
	int fd = 0;

	fd = GetFDFromName(name);
	if(fd == -1)
	{
		return -1;
	} 
	(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount)--;

	if(UFDTArr[fd].ptrfiletable->ptrinode->LinkCount == 0)
	{
		UFDTArr[fd].ptrfiletable->ptrinode->FileType = 0;
		//free(UFDTArr[fd].ptrfiletable->ptrinode->Buffer);
		free(UFDTArr[fd].ptrfiletable);
	}
	UFDTArr[fd].ptrfiletable = NULL;
	(SUPERBLOCKobj.FreeInodes)++;
}



////////////////////////////////////////////////
// Name of the function : CloseFileByName     //
// Input parameters : One Integer             //
// return value : Void                        //
// Description of the function : No           // 
// Use : Used to Close the opened file by     //
//        File Descriptor                     //
////////////////////////////////////////////////


void CloseFileByName(int fd)
{
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	(UFDTArr[fd].ptrfiletable->ptrinode->ReferenceCount)--;
}


////////////////////////////////////////////////
// Name of the function : CloseFileByName     //
// Input parameters : One character pointer   //
// return value : Integer                     //
// Description of the function : No           // 
// Use : Used to Close the opened file by     //
//        File Name                           //
////////////////////////////////////////////////

int CloseFileByName(char *name)
{
	int i = 0;
	i = GetFDFromName(name);
	if(i==-1)
	{
		return -1;
	}
	UFDTArr[i].ptrfiletable->readoffset = 0;
	UFDTArr[i].ptrfiletable->writeoffset = 0;
	(UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
	return 0;

}


////////////////////////////////////////////////
// Name of the function : CloseAllFile        //
// Input parameters : No                      //
// return value : Void                        //
// Description of the function : No           // 
// Use : Used to Close the all opened files   //
////////////////////////////////////////////////

void CloseAllFile()
{
	int i = 0;
	while(i<50)
	{
		if(UFDTArr[i].ptrfiletable != NULL)
		{
			UFDTArr[i].ptrfiletable->readoffset = 0;
	        UFDTArr[i].ptrfiletable->writeoffset = 0;
	        (UFDTArr[i].ptrfiletable->ptrinode->ReferenceCount)--;
            break;
		}
		i++;
	}
}


////////////////////////////////////////////////
// Name of the function : LseekFile           //
// Input parameters : Three Integers          //
// return value : Integer                     //
// Description of the function : No           // 
// Use : used to shift the offset according   //
//       to the user                          //
////////////////////////////////////////////////

int LseekFile(int fd,int isize,int from)
{
	if((fd<0)||(from > 2))
	{
		return -1;
	}
	if(UFDTArr[fd].ptrfiletable == NULL)
	{
		return -1;
	}

	if((UFDTArr[fd].ptrfiletable->mode == READ)||(UFDTArr[fd].ptrfiletable->mode==READ + WRITE))
	{
		if(from == CURRENT)
		{
			if(((UFDTArr[fd].ptrfiletable->readoffset)+isize) > UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)
			{
				return -1;
			}
			if(((UFDTArr[fd].ptrfiletable->readoffset)+isize) < 0)
			{
				return -1;
			}
			(UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->readoffset) + isize;
		}
	
	   else if(from == START)
	   {
		    if(isize > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
		    {
			    return -1;
		    }
		    if(isize<0)
		    {
			    return -1;
		    }
		    (UFDTArr[fd].ptrfiletable->readoffset) = isize;
	    }
	   else if(from == END)
	   {
		    if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize > MAXFILESIZE)
		    {
			    return -1;
		    }
		    if(((UFDTArr[fd].ptrfiletable->readoffset) + isize) < 0)
		    {
			    return -1;
		    }
		    (UFDTArr[fd].ptrfiletable->readoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;
	    }
    }

    else if(UFDTArr[fd].ptrfiletable->mode == WRITE)
    {
    	if(from == CURRENT)
    	{
    		if(((UFDTArr[fd].ptrfiletable->writeoffset)+isize) > MAXFILESIZE)
    		{
    			return -1;
    		}
    		if(((UFDTArr[fd].ptrfiletable->writeoffset)+isize) < 0)
    		{
    			return -1;
    		}
    		if(((UFDTArr[fd].ptrfiletable->writeoffset)+isize) > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
    		{
    			(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = (UFDTArr[fd].ptrfiletable->writeoffset) + isize;
    		}
    		(UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->writeoffset) + isize;
    	}
    	else if(from == START)
    	{
    		if(isize > MAXFILESIZE)
    		{
    			return -1;
    		}
    		if(isize < 0)
    		{
    			return -1;
    		}
    		if(isize > (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize))
    		{
    			(UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) = isize;
    		}
    		(UFDTArr[fd].ptrfiletable->writeoffset) = isize;
    	}
    	else if(from == END)
    	{
    		if((UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize)+isize > MAXFILESIZE)
    		{
    			return -1;
    		}
    		if((UFDTArr[fd].ptrfiletable->writeoffset)+isize < 0)
    		{
    			return -1;
    		}
    		(UFDTArr[fd].ptrfiletable->writeoffset) = (UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize) + isize;
    	}
    }
}

////////////////////////////////////////////////
// Name of the function : ls_file             //
// Input parameters : Nothing                 //
// return value : Void                        //
// Description of the function : No           // 
// Use : Used to Display the information      //
//       eg.File size,Inode number,etc..      //
////////////////////////////////////////////////

void ls_file()
{
	int i = 0;
	PINODE temp = head;

	if(SUPERBLOCKobj.FreeInodes == MAXINODE)
	{
		printf("ERROR : There are no files\n");
		return;
	}

	printf("\nFile Name\tInode Number\tFile Size\tLink Count\n");
	printf("------------------------------------------------------------------\n");
	while(temp != NULL)
	{
		if((temp->FileType) !=0 )
		{
			printf("%s\t\t%d\t\t%d\t\t%d\n",temp->FileName,temp->InodeNumber,temp->FileActualSize,temp->LinkCount);
		}
		temp  = temp->next;
	}
	printf("------------------------------------------------------------------\n");
}

////////////////////////////////////////////////
// Name of the function : fstat_file          //
// Input parameters : One Integer             //
// return value : Integer                     //
// Description of the function : No           // 
// Use : Used to Display the information of   //
//        File according to file Descriptor   //
////////////////////////////////////////////////

int fstat_file(int fd)
{
	PINODE temp = head;
	int i = 0;

	if(fd < 0)
	{
		return -1;
	}
	if(UFDTArr[fd].ptrfiletable == NULL)
	{
		return -2;
	}

	temp = UFDTArr[fd].ptrfiletable->ptrinode;

	printf("\n----------Statical information about file----------\n");
	printf("File Name : %s\n",temp->FileName);
	printf("Inode Number : %d\n",temp->InodeNumber);
	printf("File Size : %d\n",temp->FileSize);
	printf("Actual File Size : %d\n",temp->FileActualSize);
	printf("Link Count : %d\n",temp->LinkCount);
	printf("Reference Count : %d\n",temp->ReferenceCount);

	if(temp->Permission == 1)
	{
		printf("File Permission : Read Only\n");
	}
	else if(temp->Permission == 2)
	{
		printf("File Permission : Write\n");
	}
	else if(temp->Permission == 3)
	{
		printf("File Permission : Read & Write\n");
	}
	printf("-------------------------------------------------------\n\n");
	return 0;
}

////////////////////////////////////////////////
// Name of the function : stat_file           //
// Input parameters : One character Pointer   //
// return value : Integer                     //
// Description of the function : No           // 
// Use : Used to Display the information of   //
//        File according to file Name         //
////////////////////////////////////////////////

int stat_file(char *name)
{
	PINODE temp = head;
	int i = 0;

	
	if(name == NULL)
	{
		return -1;
	}
	while(temp != NULL)
	{
		if(strcmp(name,temp->FileName) == 0)
		{
			break;
		}
		temp = temp->next;
	}
	if(temp == NULL)
	{
		return -2;
	}

	printf("\n----------Statical information about file----------\n");
	printf("File Name : %s\n",temp->FileName);
	printf("Inode Number : %d\n",temp->InodeNumber);
	printf("File Size : %d\n",temp->FileSize);
	printf("Actual File Size : %d\n",temp->FileActualSize);
	printf("Link Count : %d\n",temp->LinkCount);
	printf("Reference Count : %d\n",temp->ReferenceCount);

	if(temp->Permission == 1)
	{
		printf("File Permission : Read Only\n");
	}
	else if(temp->Permission == 2)
	{
		printf("File Permission : Write\n");
	}
	else if(temp->Permission == 3)
	{
		printf("File Permission : Read & Write\n");
	}
	printf("-------------------------------------------------------\n\n");
	return 0;
}

////////////////////////////////////////////////
// Name of the function : truncate_file       //
// Input parameters : One Integer Pointer     //
// return value : Integer                     //
// Description of the function : No           // 
// Use : Used to Display the information of   //
//        File according to file Descriptor   //
////////////////////////////////////////////////

int truncate_File(char *name)
{
	int fd = GetFDFromName(name);
	if(fd == -1)
	{
		return -1;
	}
	memset(UFDTArr[fd].ptrfiletable->ptrinode->Buffer,0,1024);
	UFDTArr[fd].ptrfiletable->readoffset = 0;
	UFDTArr[fd].ptrfiletable->writeoffset = 0;
	UFDTArr[fd].ptrfiletable->ptrinode->FileActualSize = 0;

}


int main()
{
	char *ptr = NULL;
	int ret = 0,fd = 0,count = 0;
	char command[4][80],str[80],arr[1024];

	InitaliseSuperBlock();
	CreateDILB();

	while(1)
	{
		fflush(stdin);
		strcpy(str,"");

		printf("\nMarvellous VFS : $ ");

		fgets(str,80,stdin);

		count = sscanf(str,"%s %s %s %s",command[0],command[1],command[2],command[3]);

		if(count == 1)
		{
			if(strcmp(command[0],"ls") == 0)
			{
				ls_file();
			}
			else if(strcmp(command[0],"closeall") == 0)
			{
				CloseAllFile();
				printf("All Files Closed Successfully\n");
				continue;
			}
			else if(strcmp(command[0],"clear") == 0)
			{
				system("clear");
				continue;
			}
			else if(strcmp(command[0],"help") == 0)
			{
				DisplayHelp();
				continue;
			}
			else if(strcmp(command[0],"exit") == 0)
		    {
				printf("Terminating the Marvellous Virtual File System\n");
				break;
			}
			else
			{
				printf("\nERROR : Command not found!!!\n");
				continue;
			}

		}
		else if(count == 2)
		{
			 if(strcmp(command[0],"stat") == 0)
			{
				ret = stat_file(command[1]);
				if(ret == -1)
				{
					printf("ERROR : Incorrect Parameters\n");
				}
				if(ret == -2)
				{
					printf("ERROR : There is no such file\n");
				}
				
				continue;
			}
			else if(strcmp(command[0],"fstat") == 0)
			{
				ret = fstat_file(atoi(command[1]));
				if(ret == -1)
				{
					printf("ERROR : Incorrect Parameters\n");
				}
				if(ret == -2)
				{
					printf("ERROR : There is no such file\n");
				}
		
				continue;
			}
			else if(strcmp(command[0],"close") == 0)
			{
				ret = CloseFileByName(command[1]);
				if(ret == -1)
				{
					printf("ERROR : There is no such file\n");
				}
				
				continue;
			}
			else if(strcmp(command[0],"rm") == 0)
			{
				ret = rm_file(command[1]);
				if(ret == -1)
				{
					printf("ERROR : There is no such file\n");
				}
				continue;
			}
			else if(strcmp(command[0],"man") == 0)
			{
				man(command[1]);
			}
			else if(strcmp(command[0],"write") == 0)
			{
				fd = GetFDFromName(command[1]);
				if(fd == -1)
				{
					printf("ERROR : The file that you want to opened was not created, so first create it and then write the content\n");
					continue;
				}
				printf("Enter the data\n");
				scanf("%[^'\n']s",arr);

				ret = strlen(arr);
				if(ret == 0)
				{
					printf("Error : Incorrect Parameter\n");
					continue;
				}
				ret = WriteFile(fd,arr,ret);
				if(ret == -1)
				{
				    printf("Permission denied\n");
				}
				if(ret == -2)
				{
				    printf("There is no sufficient memory to write\n");
				}
				if(ret == -3)
				{
				    printf("It is not regular file\n");
				}
			}
			else if(strcmp(command[0],"truncate") == 0)
			{
				ret = truncate_File(command[1]);
				if(ret == -1)
				{
					printf("ERROR : Incorrect Parameter\n");
				}
				
			}
            else
		    {
				printf("\nERROR : Command Not Found\n");
				continue;
			}

		}
		else if(count == 3)
		{
			 if(strcmp(command[0],"create") == 0)
			{
				ret = CreateFile(command[1],atoi(command[2]));
				if(ret >= 0)
				{
					printf("File is Successfully created with file descriptor : %d\n",ret);
				}
				if(ret == -1)
				{
					printf("ERROR : Incorrect Parameters\n");
				}
				if(ret == -2)
				{
					printf("ERROR : There is no inodes\n");
				}
				if(ret == -3)
				{
					printf("ERROR : File Already exists\n");
				}
				if(ret == -4)
				{
					printf("ERROR : Memory allocation failure\n");
				}
				continue;
			}
			else if(strcmp(command[0],"open") == 0)
			{
				ret = OpenFile(command[1],atoi(command[2]));
				if(ret >= 0)
				{
					printf("File is Successfully opened with file descriptor :%d\n",ret);
				}
				if(ret == -1)
				{
					printf("ERROR : Incorrect Parameters\n");
				}
				if(ret == -2)
				{
					printf("ERROR : File Not Present\n");
				}
				if(ret == -3)
				{
					printf("ERROR : Permission denied\n");
				}
				continue;
			}
			else if(strcmp(command[0],"read") == 0)
			{
				fd = GetFDFromName(command[1]);
				
				if(fd == -1)
				{
					printf("ERROR : The file that you want to opened was not created, so first create it and then read the content\n");
					continue;
				}
				ptr = (char *)malloc(sizeof(atoi(command[2]))+1);
				if(ptr == NULL)
				{
					printf("Error : Memory allocation failure\n");
					continue;
				}
				ret = ReadFile(fd,ptr,atoi(command[2]));

				if(ret == -1)
				{
					printf("ERROR : File not existing\n");
				}
				if(ret == -2)
				{
					printf("ERROR : Permission denied\n");
				}
				if(ret == -3)
				{
					printf("ERROR : Reached end of file\n");
				}
				if(ret == -4)
				{
					printf("ERROR : It is not regular file\n");
				}
				if(ret == 0)
				{
					printf("File Empty\n");
				}
				if(ret > 0)
				{
					write(2,ptr,ret);
				}
				continue;
			}
			else
			{
				printf("\nERROR : Command Not Found !!!\n");
				continue;
			}
		}
		else if(count == 4)
		{
			if(strcmp(command[0],"lseek") == 0)
			{
		       	fd = GetFDFromName(command[1]);
			    if(fd == -1)
			    {
				    printf("Incorrect Parameter\n");
				    continue;
			    }
			    ret = LseekFile(fd,atoi(command[2]),atoi(command[3]));
			    if(ret == -1)
			    {
			    	printf("ERROR : Unable to perform lseek\n");
		      	}
		    }
            else
		    {
		        printf("\nERROR : Command Not Found !!!\n");
		        continue;
		    }
        }
		else
		{
		    printf("\nERROR : Command Not Found !!!\n");
		    continue;
		}
	}

	return 0;
}