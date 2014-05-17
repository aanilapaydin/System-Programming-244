/*
*	Ali Anil Apaydin
*	091044042 
*
*	CSE244 - System Programming 
*	Homework 01
*
*	Grep function with fork
*	HW01Functions.h
*/

/*HW01FUNCTIONS*/
#ifndef HW01FUNCTIONSH
#define HW01FUNCTIONSH

/* Including Libraries */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>

#define MAXLINESIZE 4096 /* Max line size constant */


/* Function definitions */
int myUsage(int argc, char** argv);

int findOccurrenceNum(FILE* fPtrFile, char* chPtrWord,int* lines);

#endif /* HW01FUNCTIONSH */
