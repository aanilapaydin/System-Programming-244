/*
 *	Ali Anil Apaydin 
 *  CSE244_HW05_091044042.c
 *  
 * 	Whell of Fortune
 *  Server and Client
 *  with threads,semaphore 
 *  and shared memory
 *  
 *  client.c
 */

#ifndef COMMONHEADER_H
#include "commonHeader.h"
#endif

#ifndef TYPES_H
#include "types.h"
#endif

#include <sys/ioctl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

#define LINENUM 3

/* Global variables declaration */
FILEPTR fPtrClient2Server;
unsigned int uiNumOfThreads;
pid_t pidtParentPid;
FILEPTR fPtrClientLog;
pthread_attr_t pthreadattr;
int iSharedMemoryID;
key_t keyTSharedMemKey;
char *chPtrSharedMemory;
CHPTR chPtrC2SFifo = CLIENT2SERVERFIFO;

/* 
 * appendLog - prints the argument and date to
 * the client log file
 * params 
 * chPtrMessage - string that will be printed
 */ 
static void appendLog(CHPTR chPtrMessage) {
	time_t timetServerTime;
	time(&timetServerTime);
	fprintf(fPtrClientLog, "%-80s | %s", chPtrMessage,
			ctime(&timetServerTime));
	fflush(fPtrClientLog);
}
/*
 * atExit - calling from before exit program
 * kills processes and updates client log file
 */
void atExit(void) {
	char chPtrCwd[PATH_MAX], chPtrCommand[LINE_MAX], chPtrLogPath[PATH_MAX];

	
	if (getpid() != pidtParentPid)
		return;


	if (fPtrClientLog)
 		fclose(fPtrClientLog);
 
	
	sprintf(chPtrLogPath, TEMPCLIENTLOGPATH, pidtParentPid);
	getcwd(chPtrCwd, PATH_MAX);
	sprintf(chPtrCommand, "mv %s %s/client.%d.log 2>/dev/null", 
						chPtrLogPath, chPtrCwd, pidtParentPid);
	system(chPtrCommand);
	
	exit(0);
	
}
/* 
 * finalize - exit program
 */
void finalize() {
	exit(0);
}
/* 
 * signalHandler - ctrl + c signal handler
 * updates client log file according to that and 
 * exits program
 */ 
static void signalHandler() {
	
	appendLog("Ctrl+C signal detected client is terminated\n");
	fputs("Ctrl+C signal detected client is terminated\n",stderr);
	finalize();
}
/* 
 * serverDie - SIGSERVERDIE signal handler
 * updates server log file according to that and 
 * exits program
 * 
 * params
 * iSignal - integer keeps signal type declarated commonHeader.h
 */ 
static void serverDie(int iSignal) {

	appendLog("Server has died. Client is terminating.");
	fputs("Server has died. Client is terminating.",stderr);
	finalize();
}
/*
 *	function -  calling from main function for each thread
 * 	open necessary file and fifos 
 *  prints request variables to client request file
 *  reads results from server respond file
 *  prints to client log file 
 *  calculates time diffrence between calculating begin and end  
 *  updates client log file
 */
void function() {
	
	char chPtrBuffer[PIPE_BUF];
	strCommand structCommand;
	double dFunctionX;
	long int liClientTid;
	srand(time(NULL));
	keyTSharedMemKey = 1820;

	sprintf(chPtrBuffer, "Number of Threads=%d. Client tid=%lu",uiNumOfThreads,pthread_self());
	appendLog(chPtrBuffer);

	memset(chPtrBuffer, '\0', PIPE_BUF);

	/* Creating a string to send server a command */
	structCommand.parentOfClientPid = getpid();
	structCommand.clientTid = pthread_self();
	structCommand.uinumberOfThreads = uiNumOfThreads;

	/* Writing the command to well known FIFO*/
	if (!(fPtrClient2Server = fopen(chPtrC2SFifo, "wb"))) {
 		perror("Could not connect to server");
 		atExit();
 	}
 	fwrite(&structCommand, sizeof(strCommand), 1, fPtrClient2Server);
	memset(chPtrBuffer, '\0', PIPE_BUF);
	
	/* Closing the common FIFO */
	fclose(fPtrClient2Server);
	
	/* Open shared memory for server to client */
    if ((iSharedMemoryID = shmget(keyTSharedMemKey, SHAREDMEMORYSIZE, 0666)) < 0) {
        perror("Shared memory get error client");
        exit(1);
    }

    if ((chPtrSharedMemory = shmat(iSharedMemoryID, NULL, 0)) == (char *) -1) {
        perror("Shared memory attachment error");
        exit(1);
    }
	
	/* Waiting server to write directory list to FIFO and read it */
	while (sscanf(chPtrSharedMemory,"%lf %lu",&dFunctionX,&liClientTid)){
		time_t timetServerTime;
		time(&timetServerTime);
		fprintf(fPtrClientLog, "f(x)=%f Client Tid=%-55lu | %s", dFunctionX,liClientTid+(rand()%uiNumOfThreads),
				ctime(&timetServerTime));
		fflush(fPtrClientLog);
	}
}
/*
 *  clientValidRequest - controls client request arguments
 * 
 *  params
 *  numberOfThreads number of threads
 */
bool clientValidRequest(unsigned int numberOfThreads){
   
 	if(numberOfThreads <= 0)
		return false;		
	
	return true;	
}
/* 
 * Main Function - Driver main initilizes signal handlers 
 * controls client arguments
 * thread inits and calls function for send request 
 * closes files,fifos after sending,printing
 * finalizes
 */
int main(int argc, CHPTR *argv) {
	
	char chArrBuff[PATH_MAX];
	pidtParentPid = getpid();

	atexit(atExit);

	if (argc != 2) {
		fputs("Usage:[EXECUTABLE] <NUMBER OF THREADS> \n", 
				stderr);
		exit(EXITCODE_USAGE);
	}
	
	/* Signal handler for ctrl+C */
	if (signal(SIGINT, signalHandler) == SIG_ERR ) {
		fputs("An error occurred while setting a signal handler.\n", stderr);
		appendLog("An error occurred while setting a signal handler.");
		finalize();
	}
	
	pthread_attr_init(&pthreadattr);
	pthread_attr_setdetachstate(&pthreadattr, PTHREAD_CREATE_DETACHED);

	/* Signal handler for SERVERDIE */
	if (signal(SIGSERVERDIE, serverDie) == SIG_ERR ) {
		perror("Could not create temporary client log file");
		finalize();
	}
	
	sprintf(chArrBuff, TEMPCLIENTLOGPATH, pidtParentPid);

	if ((fPtrClientLog = fopen(chArrBuff, "w+")) == NULL ) {
		perror("Could not create temporary client log file");
		finalize();
	}
	memset(chArrBuff, 0, PIPE_BUF);

	appendLog("Client Started.");

	uiNumOfThreads = atoi(argv[1]);
	
	if(!(clientValidRequest(uiNumOfThreads))){
		perror("Client arguments is not valid");
		atExit();
	}

	function();

	atExit();

	return 0; /* Prevent warnings */
}
