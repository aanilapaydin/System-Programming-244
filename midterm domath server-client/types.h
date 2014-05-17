/*
 *	Ali Anil Apaydin 
 *  CSE244_MID_091044042.c
 *  
 *  doMath Server and Integral/Derivate Client
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

#endif /* TYPES_H_ */
