/*
 *	Ali Anil Apaydin 
 *  CSE244_HW05_091044042.c
 *  
 * 	Whell of Fortune
 *  Server and Client
 *  with threads,semaphore 
 *  and shared memory
 * 
 *  server.c
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
FILEPTR fPtrClient2Server;
CHPTR chPTrC2SFifo = CLIENT2SERVERFIFO;
FILEPTR fPtrServerLog;
pid_t pidtPtrClientPids[MAXCLIENTREQUEST];
int iPtrClientPidArraySize;
pid_t pidtParent;
double *dPtrBufferSize;
unsigned int uiWheelOfFortuneSize;
long int liServerUpdateTime;
pthread_attr_t pthreadattr;
int iClientPidArraySize;
int iSharedMemoryID;
key_t keyTSharedMemKey;
char *chPtrSharedMemory;


/*
 * atExit - calling from before exit program
 * kills processes and updates serverlog file
 */
void atExit(void) {
	char chPtrCwd[PATH_MAX];
	char chPtrCommand[PATH_MAX];
	int iCtr;

	if (getpid() != pidtParent)
		return;

	
	for (iCtr = 0; iCtr < iClientPidArraySize; ++iCtr)
		kill(pidtPtrClientPids[iCtr], SIGSERVERDIE);

	fclose(fPtrServerLog);

	
	fclose(fPtrClient2Server);
	unlink(chPTrC2SFifo);
	free(dPtrBufferSize);
	
	getcwd(chPtrCwd, PATH_MAX);
	strcat(chPtrCwd, "/server.log");
	sprintf(chPtrCommand, "mv %s %s 2>/dev/null", TEMPSERVERLOGPATH, chPtrCwd);
	system(chPtrCommand);

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
static void function(double *dPtrBuffer,unsigned int numberOfThreads,pthread_t clientTid)
{
	int iCtr;
	CHPTR chPtrBuffer;
	char chPtrTemp[255];
	STRTIMESPEC strTimespecTimeBegin,strTimespecTimeEnd;
	double average=0;
	int randIndex;
	srand(time(NULL));
    keyTSharedMemKey = 1820;

	/* Open shared memory for server to client */
    if ((iSharedMemoryID = shmget(keyTSharedMemKey, SHAREDMEMORYSIZE, IPC_CREAT | 0666)) < 0) {
        perror("Shared memory get error server");
        exit(1);
    }

    if ((chPtrSharedMemory = shmat(iSharedMemoryID, NULL, 0)) == (char *) -1) {
        perror("Shared memory attachment error");
        exit(1);
    }

	printf("%s\n","Calculation");
	while(1){	
		
		chPtrBuffer=chPtrSharedMemory;	
		
		clock_gettime(CLOCK_MONOTONIC_RAW, &strTimespecTimeBegin);
		clock_gettime(CLOCK_MONOTONIC_RAW, &strTimespecTimeEnd);
		randIndex=rand()%uiWheelOfFortuneSize;
		for(;
			(timevalDiffInNSec(&strTimespecTimeBegin,&strTimespecTimeEnd)/MILLION)<=liServerUpdateTime
			;randIndex=(randIndex+1)/uiWheelOfFortuneSize){				
			
			dPtrBuffer[randIndex] = pow((double)(1-dPtrBuffer[randIndex]),1.00/3);
			sprintf(chPtrBuffer,"%f %lu",dPtrBuffer[randIndex],pthread_self());
			clock_gettime(CLOCK_MONOTONIC_RAW, &strTimespecTimeEnd);	
		}
		
		for(iCtr=0;iCtr<uiWheelOfFortuneSize;iCtr++){
			average+=dPtrBuffer[iCtr];
		}
		
		average=average/uiWheelOfFortuneSize;
		sprintf(chPtrTemp,"Average= %f",average);
		appendLog(chPtrTemp);
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
 * childWork - calling from main function for each thread
 * opens necessary files and fifo for each thread
 * calls function for calculation
 * calculates calculation time difference
 * updates server log file
 * 
 * params
 * voidptrStructData - structure keeps each thread infos 
 * it is void pointer because of pthread_create function
 */
void* childWork(void *voidptrStructData) {
	
	
	dPtrBufferSize = calloc(sizeof(double),uiWheelOfFortuneSize+1);
	wheelOfFortuneGenerate(dPtrBufferSize);	  

	function(dPtrBufferSize,((strCommand *)voidptrStructData)->uinumberOfThreads,
							 ((strCommand *)voidptrStructData)->clientTid);
	return 0;
}
/*
 *  serverValidRequest - controls server arguments
 * 
 *  params
 *  uiBufferSize = unsigned int Wheel of Fortune BufferSize
 *  liRequestTime = long int server respond time 
 */
bool serverValidRequest(unsigned int uiBufferSize, long int liRequestTime){
   
 	if(uiBufferSize<=0 || liRequestTime<=0)
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
	char chArrBuff[PIPE_BUF];
	pthread_t pthreadtChildTid;
	strCommand structCommand;
	char chPtrLogBuffer[LINE_MAX];
	pthread_attr_init(&pthreadattr);
	pthread_attr_setdetachstate(&pthreadattr, PTHREAD_CREATE_DETACHED);
	pidtParent = getpid();
	iClientPidArraySize = 0;
	atexit(atExit);

	if (argc != 3) {
		fputs("Usage:[EXECUTABLE] <WHELL OF FORTUNE SIZE> <UPDATETIME MILISECONDS> \n", stderr);
		exit(EXITCODE_USAGE);
	}
	
	uiWheelOfFortuneSize = atoi(argv[1]);
	liServerUpdateTime = atol(argv[2]);
	
	if(!(serverValidRequest(uiWheelOfFortuneSize,liServerUpdateTime))){
		perror("Server arguments is not valid");
		finalize();
	}

	if (signal(SIGINT, signalHandler) == SIG_ERR ) {
		fputs("An error occurred while setting a signal handler.\n", stderr);
		appendLog("An error occurred while setting a signal handler.");
		finalize(EXITCODE_SIGNALHANDLING);
	}

	if ((fPtrServerLog = fopen(TEMPSERVERLOGPATH, "w+")) == NULL ) {
		perror("Could not create temporary server log file");
		finalize(EXITCODE_LOGFILE);
	}
	

	/* Create the FIFO (named pipe) */
	mkfifo(chPTrC2SFifo, 0666);
	while (1) {
		/* Open, read, and display the message from the FIFO */
		if((fPtrClient2Server = fopen(chPTrC2SFifo, "rb")) == 0) {
			appendLog("Could not open common fifo. Terminating");
			exit(EXITCODE_FIFOOPEN);
		}

		if (fread(&structCommand, sizeof(strCommand), 1,
						fPtrClient2Server)) {
			sprintf(chPtrLogBuffer,
					"Client connected tid = %lu, Number Of threads = %u",
					structCommand.clientTid,
					structCommand.uinumberOfThreads);
					
			appendLog(chPtrLogBuffer);
			memset(chPtrLogBuffer, '\0', LINE_MAX);
			
			if (pthread_create(&pthreadtChildTid, &pthreadattr, childWork,
					(void *) &structCommand)) {
				appendLog("Error on creating thread");
			} else {
				/* Assign current child's thread id to childPidArraySize */
				pidtPtrClientPids[iClientPidArraySize++] = structCommand.parentOfClientPid;
			}
			
			/* Clean buf from any data */
			memset(chArrBuff, 0, PIPE_BUF);	
		}
		fclose(fPtrClient2Server);
	}
	pthread_attr_destroy(&pthreadattr);
	pthread_exit(EXIT_SUCCESS);

	
	return 0;
}
