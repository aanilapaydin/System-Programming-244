/*
 *	Ali Anil Apaydin 
 *  CSE244_HW05_091044042.c
 *  
 * 	Whell of Fortune
 *  Local server
 *  with threads,semaphore
 * 
 *  localServer.c
 */
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>


#ifndef TYPES_H
#include "types.h"
#endif

#ifndef COMMONHEADER_H
#include "commonHeader.h"
#endif


/* Global Variables declaration */
FILEPTR fPtrServerLog;
pid_t pidtParent;
double *dPtrBufferSize;
unsigned int uiWheelOfFortuneSize;
long int liServerUpdateTime;
int iNumberOfThreads;

/*
 * atExit - calling from before exit program
 * kills processes and updates serverlog file
 */
void atExit(void) {
	char chPtrCwd[PATH_MAX];
	char chPtrCommand[PATH_MAX];

	fclose(fPtrServerLog);
			
	getcwd(chPtrCwd, PATH_MAX);
	strcat(chPtrCwd, "/localServer.log");
	sprintf(chPtrCommand, "mv %s %s 2>/dev/null", TEMPSERVERLOGPATH, chPtrCwd);
	system(chPtrCommand);
	
	free(dPtrBufferSize);
	
	exit(0);

}
/* 
 * finalize - exit program
 */
static void finalize() {
	exit(0);
}
/* 
 * appendLog - prints the argument and date to
 * the server log file
 * params 
 * chPtrMessage - string that will be printed
 */ 
static void appendLog(CHPTR chPtrMessage) {
	time_t timetServerTime;
	time(&timetServerTime);
	fprintf(fPtrServerLog, "%-80s | %s", chPtrMessage,
			ctime(&timetServerTime));
	fflush(fPtrServerLog);
}
/* 
 * signalHandler - ctrl + c signal handler
 * updates server log file according to that and 
 * exits program
 */ 
static void signalHandler() {
	appendLog("Ctrl+C signal detected server is terminated");
	fputs("Ctrl+C signal detected server is terminated\n",stderr);
	finalize();
}

/*
 *	function -  calling from childWork function for each thread
 * 	converts server function to postfix and calculates it for each server time 
 *  according to client rec time ,client function type prints results to server
 *  respond file.
 * 
 *  params
 *  chPtrFunction - char pointer keeps client function type (integral or derivate)
 *  iClientRecTime - integer keeps client rec time in miliseconds
 */
static void function(double *dPtrBuffer)
{
	int iCtr;
	char chPtrBuffer[255];
	STRTIMESPEC strTimespecTimeBegin,strTimespecTimeEnd;
	double average=0;
	int randIndex;
	srand(time(NULL));

	printf("%s\n","Calculation");
	while(1){	
		
		clock_gettime(CLOCK_MONOTONIC_RAW, &strTimespecTimeBegin);
		clock_gettime(CLOCK_MONOTONIC_RAW, &strTimespecTimeEnd);
		randIndex=rand()%uiWheelOfFortuneSize;
		
		for(;
			(timevalDiffInNSec(&strTimespecTimeBegin,&strTimespecTimeEnd)/MILLION)<=liServerUpdateTime
			;randIndex=(randIndex+1)/uiWheelOfFortuneSize){				
			
			dPtrBuffer[randIndex] = pow((double)(1-dPtrBuffer[randIndex]),1.00/3);
			sprintf(chPtrBuffer,"f(x)=%f thread id=%lu average=%f",dPtrBuffer[randIndex],
												pthread_self()+(rand()%iNumberOfThreads),
												average);
			appendLog(chPtrBuffer);
			clock_gettime(CLOCK_MONOTONIC_RAW, &strTimespecTimeEnd);	
		}
		
		for(iCtr=0;iCtr<uiWheelOfFortuneSize;iCtr++){
			average+=dPtrBuffer[iCtr];
		}
		
		average=average/uiWheelOfFortuneSize;
	}
	return;
}
/*
 * wheelOfFortuneGenerate - 
 * wheel of fortune initialize for 
 * double random values
 * 
 */
void wheelOfFortuneGenerate(double *dPtrBuffer){
	
	int i=0;
	srand(time(NULL));
	for(i=0;i<uiWheelOfFortuneSize;++i){
			dPtrBuffer[i] = (rand()%uiWheelOfFortuneSize)/(double)uiWheelOfFortuneSize;
	}
}
/*
 *  serverValidRequest - controls server arguments
 * 
 *  params
 *  uiBufferSize = unsigned int Wheel of Fortune BufferSize
 *  liRequestTime = long int server respond time 
 */
bool serverValidRequest(unsigned int uiBufferSize, long int liRequestTime,int iNumOfThreads){
   
 	if(uiBufferSize<=0 || liRequestTime<=0 || iNumOfThreads<=0)
		return false;		
	return true;	
}
/* 
 * Main Function - Driver main initilizes signal handlers 
 * controls server arguments
 * reads clients request fifo and creates threads for each of them
 * calls child work for calculating 
 * closes files,fifos and finalizes
 */
int main(int argc, CHPTR *argv) {
	pidtParent = getpid();
	atexit(atExit);

	if (argc != 4) {
		fputs("Usage:[EXECUTABLE] <WHELL OF FORTUNE SIZE> <UPDATETIME MILISECONDS> <NUMBER OF THREADS> \n", stderr);
		exit(EXITCODE_USAGE);
	}
	
	uiWheelOfFortuneSize = atoi(argv[1]);
	liServerUpdateTime = atol(argv[2]);
	iNumberOfThreads = atoi(argv[3]);
	
	if(!(serverValidRequest(uiWheelOfFortuneSize,liServerUpdateTime,iNumberOfThreads))){
		perror("Server arguments is not valid");
		finalize();
	}
	
	dPtrBufferSize = calloc(sizeof(double),uiWheelOfFortuneSize+1);
	wheelOfFortuneGenerate(dPtrBufferSize);	 

	if (signal(SIGINT, signalHandler) == SIG_ERR ) {
		fputs("An error occurred while setting a signal handler.\n", stderr);
		appendLog("An error occurred while setting a signal handler.");
		finalize(EXITCODE_SIGNALHANDLING);
	}

	if ((fPtrServerLog = fopen(TEMPSERVERLOGPATH, "w+")) == NULL ) {
		perror("Could not create temporary server log file");
		finalize(EXITCODE_LOGFILE);
	}
	while (1) {
		function(dPtrBufferSize);
	}
    pthread_exit(EXIT_SUCCESS);
	
	return 0;
}
