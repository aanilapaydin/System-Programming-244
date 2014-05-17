/*
*	Ali Anil Apaydin
*	091044042 
*
*	CSE244 - System Programming 
*	Homework 01
*
*	Grep function with fork
*	grepM.c
*/

/* Including libraries*/
#include "HW01Functions.c"
#include "childWait.h"

/* Main function*/
int main(int argc, char** argv) {
	
	/* Variables definitons*/
	FILE* fPtrInput;
	pid_t pidtIsParent;
	char* chPtrWord;
	int iCtr, iWordCount;
	int linesArray[MAXLINESIZE]; 
	int* lines = &linesArray[0];
	int i, tempLine;
	int lineOccur=0;


	/* Checking the usage */
	if (!myUsage(argc, argv))
		return 1;

	/* Printing parent's information */
	printf("%s: %d\n", "Parent pId", getpid());
	
	/* Creating child process */
	pidtIsParent = fork();

	/* Setting the current process' argument. */
	chPtrWord = argv[2];

	/* If child process*/
	if (!pidtIsParent) {

		/* Opening file */
		fPtrInput = fopen(&argv[1][1], "r");

		/* Checking whether the file was opened */
		if (fPtrInput == NULL ) {
		fprintf(stderr, "Cannot open input file.\n");
			return 2;
		}

		/* Getting the number of occurrences */
		iWordCount = findOccurrenceNum(fPtrInput, chPtrWord , lines);

		/* Printing the child processes information */
		printf("\nWord: %-20s Count: #%-5d pId: %-6d ppId:%-6d \n", chPtrWord,
			iWordCount, getpid(), getppid());
		
		/* Member initilization before the loop */ 			
		tempLine=0;
		lineOccur=0;
		
		/* Loop for lines informations */
		for(i=0;i<iWordCount;++i){
			if(i==0 && iWordCount!=1){
				 printf("Lines are:\n");
				 tempLine=lines[0];	
				 ++lineOccur;			
			}
			else if(i==0 && iWordCount==1)
				 printf("Line Number %d Number of occurrences %d\n",lines[i],lineOccur+1);
			else if(i==iWordCount-1)
				 printf("Line Number %d Number of occurrences %d\n",lines[i],lineOccur+1);
			else{
				 if(tempLine==lines[i]){
				 	++lineOccur;
				 }	
				 else{
					printf("Line Number %d Number of occurrences %d\n",lines[i-1],lineOccur);
					lineOccur=1;
					tempLine=lines[i];
				 }				
			}	
		}

		/* Safely closing the file pointer */
		fclose(fPtrInput);
		/* Child processes ends */
		return 0;
		
	}

	/* Waiting for the current child process */
		while (childWait(NULL ) > 0);

	/* Printing the parent processes information */
	printf("%s pId: %d \n", "\n\nDone parent.", getpid());
	
	/* Program ends */
	return 0;

}
