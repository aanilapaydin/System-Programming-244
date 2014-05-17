/*
 *	Ali Anil Apaydin 
 *  CSE244_HW04_091044042.c
 *  
 *  doMath Server and Integral/Derivate Client
 *  with threads
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
	char chfunction[PATH_MAX];
	long int iclientRecTime;
	pthread_t clientTid;
	pid_t parentOfClientPid;
} strCommand;


#endif /* TYPES_H_ */
