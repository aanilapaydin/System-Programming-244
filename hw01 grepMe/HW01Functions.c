/*
*	Ali Anil Apaydin
*	091044042 
*
*	CSE244 - System Programming 
*	Homework 01
*
*	Grep function with fork
*	HW01Functions.c
*/

/* Including libraries */
#include "HW01Functions.h"


/* Controls sample usage and prints the appropriate usage if there's an error */
int myUsage(int argc, char** argv) {
	if (argc < 3 || argc > 3 || argv[1][0] != '-') {
		fprintf(stderr,"Invalid arguments.\n Usage: ./%s -fileName \"[WORD]\" \n",
				argv[0]);
		return 0;
	}
	return 1;
}


/* Returns the number of string occurrences in file,saves the line numbers that is encountered,incase-sensitively. */
int findOccurrenceNum(FILE* fPtrFile, char* chPtrWord,int* lines) {

	/* Variables Definitions */
	int occurrence = 0;
	int lineCount = 1;
	int count = 0;
	int charPositionFirst;
	int stat, i, tempStat;
	char ch, temp, tempCount;

	/* Loop for scan the file character by character */
	for(stat=fscanf(fPtrFile,"%c",&ch);stat!=EOF;stat=fscanf(fPtrFile,"%c",&ch)){

		/* If newline char is encountered */
		if(ch=='\n')
			++lineCount;
		/* If the char that is scanned is word's first char */
		if(ch==chPtrWord[0]){
		     tempCount=1;
		     /* Save char file pointer's current position */
		     charPositionFirst = ftell(fPtrFile);
		     
		     /* Loop for compare scanned chars to word's other chars */
		     for(i=1;i<strlen(chPtrWord);++i){
		     	  tempStat=fscanf(fPtrFile,"%c",&temp);
			  if(tempStat!=EOF && temp==chPtrWord[i]){
			  	tempCount++;
			  }
                     }
		     /* If the words is encountered in the file */
		     if(tempCount==strlen(chPtrWord)){
			++occurrence;
			lines[count] = lineCount;		/* Lines array for the lines of occurrences */
			++count;
		     }
		     /* Remove the file pointer's position to the saved position */
		     fseek(fPtrFile,charPositionFirst,SEEK_SET);
		}
		
	}

	return occurrence;
}

