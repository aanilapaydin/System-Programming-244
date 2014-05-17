/*
 *	Ali Anil Apaydin 
 *  CSE244_HW04_091044042.c
 *  
 *  doMath Server and Integral/Derivate Client
 * 	with threads 
 * 
 *  doMath.c
 */
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include  <signal.h>
#include <stdlib.h>

#ifndef TYPES_H
#include "types.h"
#endif

#ifndef COMMONHEADER_H
#include "commonHeader.h"
#endif

#include "mathFunctions.c"

/* Global Variables declaration */
FILEPTR fPtrClient2Server;
CHPTR chPTrC2SFifo = CLIENT2SERVERFIFO;
STRTIMESPEC strTimespecClientConnected;
FILEPTR fPtrServerLog;
pid_t pidtPtrClientPids[MAXCLIENTREQUEST];
int iPtrClientPidArraySize;
pid_t pidtParent;
CHPTR chPtrMainFunction;
long int liServerNanoSec;
int iClientPidArraySize;

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

	if(fPtrServerLog)
 		fclose(fPtrServerLog);
 
 	if(fPtrClient2Server) {
 		fclose(fPtrClient2Server);
 		unlink(chPTrC2SFifo);
 	}

	
	getcwd(chPtrCwd, PATH_MAX);
	strcat(chPtrCwd, "/doMath.log");
	sprintf(chPtrCommand, "mv %s %s 2>/dev/null", TEMPSERVERLOGPATH, chPtrCwd);
	system(chPtrCommand);

	return;
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
	fprintf(fPtrServerLog, "%-100s | %s", chPtrMessage,
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
 *  filePtrServer2Client - file pointer points server respond file
 *  chPtrFunction - char pointer keeps client function type (integral or derivate)
 *  iClientRecTime - integer keeps client rec time in miliseconds
 */
static void function(FILEPTR filePtrServer2Client,CHPTR chPtrFunction,
					   int iClientRecTime)
{
	long double ldResult=0;
	int iPostFixStat = 0;
	long int iCount = 0;
	int iCtr;
	char chPtrBuffer[255];
	STRTIMESPEC strTimespecTimeBegin;
	clock_gettime(CLOCK_MONOTONIC_RAW, &strTimespecTimeBegin);
	globArgv = chPtrMainFunction;
	globLenOfFunc = strlen(globArgv);
	pendingTime = liServerNanoSec;
	iPostFixStat = convertToPostfix(); 
	
	if(iPostFixStat){}
	else{
		finalize();
	}

	if(!strcmp(chPtrFunction,"integral")){
		//INTEGRAL
		printf("%s\n",chPtrFunction);
		for(iCount=0,iCtr=1;iCount<iClientRecTime*MILLION;iCount+=liServerNanoSec,iCtr++){				
			currentParam=timevalDiffInNSec(&strTimespecClientConnected,&strTimespecTimeBegin);
			ldResult=mathFunction(chPtrFunction);
			if(ldResult==-1)
				break;
			sprintf(chPtrBuffer,"\n%d.iteration %s of %s= %Lf",iCtr,chPtrFunction,
																chPtrMainFunction,ldResult);
			fprintf(filePtrServer2Client,"%s",chPtrBuffer);
			clock_gettime(CLOCK_MONOTONIC_RAW, &strTimespecTimeBegin);
		}
	}		
	else{
		//DERIVATE
		printf("%s\n",chPtrFunction);
		for(iCount=0,iCtr=1;iCount<iClientRecTime*MILLION;iCount+=liServerNanoSec,iCtr++){	
			currentParam=timevalDiffInNSec(&strTimespecClientConnected,&strTimespecTimeBegin);
			ldResult=mathFunction(chPtrFunction);
			if(ldResult==-1)
				break;
			sprintf(chPtrBuffer,"\n%d.iteration %s of %s= %Lf",iCtr,chPtrFunction,
																chPtrMainFunction,ldResult);
			fprintf(filePtrServer2Client,"%s",chPtrBuffer);
			clock_gettime(CLOCK_MONOTONIC_RAW, &strTimespecTimeBegin);
		}
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
	
	FILEPTR fPtrServer2Client; 
	char chPtrPath[PATH_MAX];
	char chPtrLogBuffer[LINE_MAX];
	STRTIMESPEC strTimespecTimeBegin, strTimespecTimeEnd;

	clock_gettime(CLOCK_MONOTONIC_RAW, &strTimespecClientConnected);
	sprintf(chPtrPath, SERVER2CLIENTFIFOPATTERN, ((strCommand *)voidptrStructData)->clientTid);
	
	if ((fPtrServer2Client = fopen(chPtrPath, "w")) == NULL ) {
  		perror("Could not open server to client fifo.");
  		finalize(EXITCODE_FIFOOPEN);
  	}   

	clock_gettime(CLOCK_MONOTONIC_RAW, &strTimespecTimeBegin);

	function(fPtrServer2Client,((strCommand *)voidptrStructData)->chfunction,
									((strCommand *)voidptrStructData)->iclientRecTime);
	
	clock_gettime(CLOCK_MONOTONIC_RAW, &strTimespecTimeEnd);

	sprintf(chPtrLogBuffer, "Calculation completed for thread id = %lu in %ld msecs",
			((strCommand *) voidptrStructData)->clientTid, 
			timevalDiffInMicSec(&strTimespecTimeBegin, &strTimespecTimeEnd));
	
	appendLog(chPtrLogBuffer);


#ifndef NDEBUG
	sleep(1);
#endif
	char chtempBuf[255];
	
	sprintf(chtempBuf, "\n\n\t\tTime difference=%ld nanoseconds",
			timevalDiffInNSec(&strTimespecTimeBegin, &strTimespecTimeEnd));
	
	fprintf(fPtrServer2Client, "%s",chtempBuf);
	
	fclose(fPtrServer2Client);
	return 0;
}
/*
 *  serverValidRequest - controls server arguments
 * 
 *  params
 *  chPtrRequest - char pointer keeps server's main function
 *  liRequestTime - long integer keeps server's time in nanoseconds
 */
bool serverValidRequest(CHPTR chRequest, long int liRequestTime){
   
 	if(!((strcmp(chRequest,""))&&(liRequestTime>0)))
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
	pthread_attr_t pthreadattr;

	pthread_attr_init(&pthreadattr);
	pthread_attr_setdetachstate(&pthreadattr, PTHREAD_CREATE_DETACHED);

	pidtParent = getpid();
	iClientPidArraySize = 0;
		
	atexit(atExit);

	if (argc != 3) {
		fputs("Usage:[EXECUTABLE] <FUNCTION> <DIFFTIME NANOSECONDS> \n", stderr);
		exit(EXITCODE_USAGE);
	}
	
	chPtrMainFunction = argv[1];
	liServerNanoSec = atol(argv[2]);
	
	if(!(serverValidRequest(chPtrMainFunction,liServerNanoSec))){
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
	
	appendLog("Server started");

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
					"Client connected tid = %lu, Function = %s ClientRecTime = %ld",
					structCommand.clientTid,
					structCommand.chfunction,
					structCommand.iclientRecTime);
					
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
	
	unlink(chPTrC2SFifo);
	finalize(0);
	
	return 0;
}
