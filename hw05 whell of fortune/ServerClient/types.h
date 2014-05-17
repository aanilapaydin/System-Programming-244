/*
 *	Ali Anil Apaydin 
 *  CSE244_HW05_091044042.c
 *  
 * 	Whell of Fortune
 *  Server and Client
 *  with threads,semaphore 
 *  and shared memory
 * 
 *  types.h
 */

#ifndef TYPES_H_
#define TYPES_H_

#include <dirent.h>

typedef char * CHPTR;
typedef int FILEDESC;
typedef FILE * FILEPTR;
typedef struct dirent* DIRENTP;
typedef DIR* DIRP;
typedef struct stat STRSTAT;
typedef struct timeval STRTIMEVAL;
typedef struct timeval* STRTIMEVALPTR;
typedef struct timespec STRTIMESPEC;
typedef struct timespec* STRTIMESPECPTR;
typedef struct winsize WINSIZE_T;
typedef struct termios TERMINALATTRIBUTES;

typedef struct {
	unsigned int uinumberOfThreads;
	pthread_t clientTid;
	pid_t parentOfClientPid;
} strCommand;


#endif /* TYPES_H_ */
