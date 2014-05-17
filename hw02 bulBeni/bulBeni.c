/*
*	Ali Anil Apaydin
*	091044042 
*
*	CSE244 - System Programming 
*	Homework 02
*
*	Grep function in all text 
*	files that are in given directory.
*	bulBeni.c
*/

/* Including libs */
#include "HW02Functions.c"

/* Main Function */
int main(int argc, char** argv) {
	
	/* Variables definitons*/
	FILEPTR fPtrInput,fPtrOutput;
	pid_t pidtIsParent;
	char* chPtrWord;
	int iWordCount;
	int linesArray[MAXLINESIZE]; 
	int* lines = &linesArray[0];
	int i,j,totalFilesNum,totalTextFilesNum,tempLine;
	int lineOccur=0;
	CHPTR dirName;
	char chPtrFileName[LINESIZE];
	FILEPTR fPtrFile;
	pid_t pidParent = getpid();
	char filesFound[MAXLINESIZE][LINESIZE];	
	int txtFilesIndex[LINESIZE];
	int stat;
	char tempChar[LINESIZE],ch,tch;
	char printLineStr[100]; 
	int len;
	int printLineCount;

	
	/* Check Usage exclude print count */
	if(!myUsage(argc, argv))
		return 1;

	/* 
	 * Assignment directory name and print count 
	 * Check print count is vaild
	 */
	dirName = argv[1];
	printLineCount=atoi(argv[5]);
	if(printLineCount<=0){
		myUsage(printLineCount,argv);
		return 1;
	}
		
	
	puts("Parent");
	printf("%-8s %s\n","PID", "FILE NAME");
	puts("==================================================================");
	/* Find and record files,directories */ 
	recDirWithFork(dirName,1,getpid());
	
	/* Parent will print the contents of its own file and remove it */
	if(getpid() == pidParent) {
		getFileName(chPtrFileName, dirName);
		fPtrFile = fopen(chPtrFileName, "r");
		
		/* Assign loop total founded files and directories */
		for(j=0,stat=fscanf(fPtrFile,"%s",filesFound[j]);stat!=EOF;++j,stat=fscanf(fPtrFile,"%s",filesFound[j])){}
			totalFilesNum=j;

		/* Assign loop total founded text files */
		totalTextFilesNum=0;
		for(j=0;j<totalFilesNum;++j){
			len=strlen(filesFound[j]);
    			if (len > 4 && strcmp(filesFound[j] + len - 4, ".txt") == 0){
					txtFilesIndex[totalTextFilesNum]=j;
					totalTextFilesNum++;	    
			}		
		}
		/* Printing total founded text files to stdin */
		puts("Total text files found:");
		for(j=0;j<totalTextFilesNum;++j){
			strcpy(tempChar,argv[1]);
			strcat(tempChar,filesFound[txtFilesIndex[j]]);
			strcpy(filesFound[txtFilesIndex[j]],tempChar);
			printf("%s\n",filesFound[txtFilesIndex[j]]);
		}
		/* 
		 * Close filepointer and delete total founded files 
		 * and directories file 
		 */	
		fclose(fPtrFile);
		unlink(chPtrFileName);
	
		/* 
		 * Record loop for word counts and lines 
		 * to the linesOutput.txt file 
		 */
		for(j=0;j<totalTextFilesNum;++j){
			/* Creating child process */
			pidtIsParent = fork();
			/* Setting the current process' argument. */
			chPtrWord = argv[3];
			
			/* If child process*/
			if (!pidtIsParent) {
				/* Opening files */
				fPtrInput = fopen(filesFound[txtFilesIndex[j]], "r");
				fPtrOutput = fopen("linesOutput.txt","a+");
	
				/* Checking whether the files was opened */
				if (fPtrInput == NULL) {
					fprintf(stderr, "Cannot open input file.\n");
					return 2;
				}
				if(fPtrOutput==NULL){
					fprintf(stderr, "Child process can not update the output file.\n");
					return 2;
				}

				/* Getting the number of occurrences in files*/
				iWordCount = findOccurrenceNum(fPtrInput, chPtrWord , lines);
					
				/* Printing to file the child processes information */
				fprintf(fPtrOutput,"Word: %-20s Count: #%-5d pId: %-6d ppId:%-6d \n", chPtrWord,
					iWordCount, getpid(), getppid());
							
				/* Member initilization before the loop */ 			
				tempLine=0;
				lineOccur=0;
				fprintf(fPtrOutput,"File:%s\n",filesFound[txtFilesIndex[j]]);
				/* Loop for record lines informations to the linesOutput.txt*/
				for(i=0;i<iWordCount;++i){
					if(i==0 && iWordCount!=1){
						 fprintf(fPtrOutput,"Lines are:\n");
						 tempLine=lines[0];	
						 ++lineOccur;			
					}
					else if(i==0 && iWordCount==1)
						 fprintf(fPtrOutput,"Line Number %d Number of occurrences %d\n",lines[i],lineOccur+1);
					else if(i==iWordCount-1)
						 fprintf(fPtrOutput,"Line Number %d Number of occurrences %d\n",lines[i],lineOccur+1);
					else{
						 if(tempLine==lines[i]){
						 	++lineOccur;
						 }	
						 else{
							fprintf(fPtrOutput,"Line Number %d Number of occurrences %d\n",lines[i-1],lineOccur);
							lineOccur=1;
							tempLine=lines[i];
						 }				
					}	
				}
				/* Safely closing the file pointers */
				fclose(fPtrInput);
				fclose(fPtrOutput);
				/* Child processes ends */
				return 0;
		
			}
			/* Waiting for the current child process */
			while (childWait(NULL ) > 0);
		}

		/* Open linesOutput.txt files */
		fPtrOutput=fopen("linesOutput.txt","r");
		/* Checking whether the file was opened */
		if(fPtrOutput==NULL){
			fprintf(stderr, "Parent process can not open the output file.\n");
			return 2;
		}	
		/* 
		 * For separate founded text files list and
		 *  lines of word in the file 
		 */
		puts("\n\n");	
		
		/* Do-while loop print like less command in UNIX */
		do{
			/* Print lines loop*/
			for(i=1;i<=printLineCount;++i){
				if(fgets(printLineStr,100,fPtrOutput)!=NULL){
					printf("%s",printLineStr);		
				}
				/* If file is over */	
				else{
					puts("FILE IS OVER...\n");
					fclose(fPtrOutput);
					unlink("linesOutput.txt");
				
					/* Printing the parent processes information */
					printf("%s pId: %d \n", "\n\nDone parent.", getpid());
					return 0;
				}		
			}
			/* Like less command */
			printf("MORE?");
			ch=getchar();
			if(ch!='\n')
				tch=getchar();
		}while(ch!='q' && ch!='Q');
		
		/* If q or Q pressed in do-while loop prints all file*/
		while(fgets(printLineStr,100,fPtrOutput)!=NULL)
			printf("%s",printLineStr);
		
		/* File is over,safely close file pointer and */
		/* delete linesOutput.txt file */
		puts("FILE IS OVER...\n");
		fclose(fPtrOutput);
		unlink("linesOutput.txt");
		
		/* Printing the parent processes information */
		printf("%s pId: %d \n", "\n\nDone parent.", getpid());
		
	}

	/* Program ends */
	return 0;

}






	

