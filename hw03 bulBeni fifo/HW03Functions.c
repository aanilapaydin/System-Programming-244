/*
*	Ali Anil Apaydin
*	091044042 
*
*	CSE244 - System Programming 
*	Homework 03
*
*	Grep function in all text 
*	files that are in given directory
* 	with fork,fifo.
*	HW03Functions.c
*/

/* Including libs */
#include "HW03Functions.h"

/* Usage function */ 
int myUsage(int argc, char** argv) 
{
	if (argc != 6 || strcmp(argv[2],"-g")!=0 || strcmp(argv[4],"-l")!=0) {
		printf("Invalid arguments.\n Usage: ./ProgramName [DIRECTORY PATH] -g [WORD] -l [LINE NUMBER]\n");
		return 0;
	}
	return 1;
}
/* Child wait function for parent process */ 
pid_t childWait() {
   pid_t retval;
   while (((retval = wait(NULL)) == -1) && (errno == EINTR)) ;
		return retval;
}
/*
 * Returns the relative path of path.
 * Eg: path = /home/user/Desktop/myfile.txt
 * for iLevel = 0 returns myfile.txt
 * for iLevel = 1 returns Desktop/myfile.txt
 *
 * Pre: iLevel >= 0
 *      path != 0
 */
CHPTR getDirName(CHPTR path, int iLevel) {
	int i = 0;
	CHPTR chPtrPos, chPtrPathCopy = malloc(sizeof(char) * strlen(path) + 1);
	
	strcpy(chPtrPathCopy, path);
	chPtrPos = chPtrPathCopy;
	while (chPtrPos && i < iLevel) {
		chPtrPos = strrchr(chPtrPathCopy, '/') - 1;
		chPtrPos[1] = '\0';
		++i;
	}
	/* Set super directories position */
	chPtrPos = path + (chPtrPos - chPtrPathCopy) + 2;

	free(chPtrPathCopy);
	return chPtrPos;
}
/*
 * Returns the file name which contains directory entries in path.
 * Concatenates the path and List.txt and copies it to
 * chPtrFileName.
 */
CHPTR getFileName(CHPTR chPtrFileName, CHPTR path) {
	strcat(chPtrFileName, path);
	if (chPtrFileName[strlen(chPtrFileName) - 1] == '/')
		chPtrFileName[strlen(chPtrFileName) - 1] = '\0';
	strcat(chPtrFileName, "List.txt");

	return path;
}
/*
 * Recursively prints the directory entries. New processed will be
 * created for each directory.
 */
void recDirWithFork(CHPTR path, int iLevel, int childPid) {
	DIRP dirp;
	DIRENTP direntpCwd;
	char chPtrParentFile[LINESIZE] = "", chPtrChildFile[LINESIZE] = "";
	CHPTR chPtrDirPath;
	CHPTR chPtrDirName = getDirName(strcat(path, "/"), iLevel);
	char chBuffer;
	int iChildPid;
	FILEPTR fPtrFile, fPtrChild;

	/*Open path*/
	getFileName(chPtrParentFile, path);
	fPtrFile = fopen(chPtrParentFile, "w");
	/* If directory can not open */
	if ((dirp = opendir(path)) == NULL ) {
		if (errno == EACCES)
			printf("Error: Permission denied for directory: %s", path);
		else
			printf("%s: %s\n", "Error: Cannot open directory", path);
		return;
	}
	
	while ((direntpCwd = readdir(dirp)) != NULL ) {
		fprintf(fPtrFile, "%s%s\n", chPtrDirName,
				direntpCwd->d_name);

		/* Filtering the current and upper directory */
		if (strcmp(direntpCwd->d_name, ".")
				&& strcmp(direntpCwd->d_name, "..")) {

			/* Is directory*/
			if (direntpCwd->d_type == DT_DIR) {

				if ((iChildPid = fork()) == -1) {
					perror("Unable to fork");
					exit(EXIT_FAILURE);
				}

				/* Allocating space for child directory */
				chPtrDirPath = malloc(
						strlen(path) + strlen(direntpCwd->d_name) + 1);
				strcpy(chPtrDirPath, path);
				strcat(chPtrDirPath, direntpCwd->d_name);

				if (iChildPid == 0) {
					/* If child process */
					recDirWithFork(chPtrDirPath, iLevel + 1, iChildPid);
					exit(EXIT_SUCCESS);
				} 
				else {
					/* If parent process*/
					/* Waiting for child */
					while (childWait(NULL) > 0);
					/* Update variables for sub directories */
					chPtrChildFile[0] = '\0';
					getFileName(chPtrChildFile, chPtrDirPath);

					fPtrChild = fopen(chPtrChildFile, "r");
					while ((chBuffer = fgetc(fPtrChild)) != EOF){
						fputc(chBuffer, fPtrFile);
					}
					fclose(fPtrChild);
					unlink(chPtrChildFile);	
				}
				free(chPtrDirPath);
			}
		}
	}
	while ((closedir(dirp) == -1) && (errno == EINTR));
	fclose(fPtrFile);
}
/* 
 * Returns the number of string occurrences in file,
 * saves the line numbers that is encountered,incase-sensitively. 
 */
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
