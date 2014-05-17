/*
*	Ali Anil Apaydin
*	091044042 
*
*	CSE244 - System Programming 
*	Homework 03
*
*	Grep function in all text 
*	files that are in given directory
*	with fork,fifo.
* 	HW03Functions.h
*/

/* HW03FUNCTIONSH */
#ifndef HW03FUNCTIONSH
#define HW03FUNCTIONSH

/* Include Libs */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>



#define LINESIZE 256     
#define MAXLINESIZE 4096 /* Max line size constant */

/* Macros definitions */

typedef struct dirent* DIRENTP;
typedef DIR* DIRP;
typedef struct stat STRSTAT;
typedef char* CHPTR;
typedef FILE* FILEPTR;


/* Functions Definitions */

int myUsage(int argc, CHPTR* argv);

int findOccurrenceNum(FILE* fPtrFile, char* chPtrWord,int* lines);

CHPTR getDirName(CHPTR path, int iLevel);

CHPTR getFileName(CHPTR chPtrFileName, CHPTR path);

void recPrintDirWithFork(CHPTR path, int iLevel, int childPid);

pid_t childWait();


#endif /* HW03FUNCTIONSH */
