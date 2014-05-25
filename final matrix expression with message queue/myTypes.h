/*
 *
 *  Created on: May 21,2014
 *  Author: Ali Anil Apaydin
 *  
 *  CSE244_FINAL_091044042
 *  
 *  Description:Matrix inverting,decomposition,
 *  	 	verification,multiplication
 *  
 *
 * 	myTypes.h
 */

#ifndef MYTYPES_H_
#define MYTYPES_H_

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

typedef int bool;
enum { false, true };

#endif /* MYTYPES_H_ */
