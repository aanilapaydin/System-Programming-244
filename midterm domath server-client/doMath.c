/*
 *	Ali Anil Apaydin 
 *  CSE244_MID_091044042.c
 *  
 *  doMath Server and Integral/Derivate Client
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

FILEDESC fdClient2Server;
CHPTR chPTrC2SFifo = CLIENT2SERVERFIFO;
STRTIMESPEC clientConnected;
FILEPTR filePtrServerLog;

int iPtrChildPids[MAXCLIENTREQUEST];
int iChildPidArraySize;

int iPtrClientPids[MAXCLIENTREQUEST];
int iClientPidArraySize;

pid_t pidtParent;


CHPTR chPtrMainFunction;
long int serverNanoSec;

void atExit(void) {
	char chPtrCwd[PATH_MAX];
	char chPtrCommand[PATH_MAX];
	int iCtr;

	if (getpid() != pidtParent)
		return;

	while (r_wait(NULL ) > 0)
		;
	/* wait for all of your children */

	fclose(filePtrServerLog);
	close(fdClient2Server);
	unlink(chPTrC2SFifo);

	for (iCtr = 0; iCtr < iClientPidArraySize; ++iCtr)
		kill(iPtrClientPids[iCtr], SIGSERVERDIE);

	for (iCtr = 0; iCtr < iChildPidArraySize; ++iCtr)
			kill(iPtrChildPids[iCtr], SIGSERVERDIE);

	getcwd(chPtrCwd, PATH_MAX);
	strcat(chPtrCwd, "/doMath.log");
	sprintf(chPtrCommand, "mv %s %s", TEMPSERVERLOGPATH, chPtrCwd);
	system(chPtrCommand);

	return;
}

static void finalize() {
	exit(SERVERDIE);
}

static void catchChildProc(int iSignal) {
	wait3(NULL, WNOHANG, NULL );
}

static void appendLog(CHPTR chPtrMessage) {
	time_t timetServerTime;
	time(&timetServerTime);
	fprintf(filePtrServerLog, "%-80s | %s", chPtrMessage,
			ctime(&timetServerTime));
	fflush(filePtrServerLog);
}

static void signalHandler() {
	appendLog("Ctrl+C signal detected server is terminated");
	fputs("Ctrl+C signal detected server is terminated\n",stderr);
	finalize();
}

static void function(FILEDESC fileDescServer2Client,CHPTR chPtrFunction,
					   int clientRecTime)
{
	long double result=0;
	int postFixStat = 0;
	long int iCount = 0;
	int i;
	char chPtrBuffer[255];
	STRTIMESPEC timeBegin;
	clock_gettime(CLOCK_MONOTONIC_RAW, &timeBegin);
	globArgv = chPtrMainFunction;
	globLenOfFunc = strlen(globArgv);
	pendingTime = serverNanoSec;
	postFixStat = convertToPostfix(); 
	
	if(postFixStat){}
	else{
		finalize();
	}

	if(!strcmp(chPtrFunction,"integral")){
		//INTEGRAL
		printf("Integral\n");
		for(iCount=0,i=1;iCount<clientRecTime*MILLION;iCount+=serverNanoSec,i++){				
			currentParam=timevalDiffInNSec(&clientConnected,&timeBegin);
			result=mathFunction(chPtrFunction);
			if(result==-1)
				break;
			sprintf(chPtrBuffer,"%d.iteration %s of %s=%Lf \n",i,chPtrFunction,
																chPtrMainFunction,result);
			
			write(fileDescServer2Client, chPtrBuffer, sizeof(chPtrBuffer));
			clock_gettime(CLOCK_MONOTONIC_RAW, &timeBegin);
		}
	}		
	else{
		//DERIVATE
		printf("derivate\n");
		for(iCount=0,i=1;iCount<clientRecTime*MILLION;iCount+=serverNanoSec,i++){	
			currentParam=timevalDiffInNSec(&clientConnected,&timeBegin);
			result=mathFunction(chPtrFunction);
			if(result==-1)
				break;
			sprintf(chPtrBuffer,"%d.iteration %s of %s=%Lf \n",i,chPtrFunction,
																chPtrMainFunction,result);
			
			write(fileDescServer2Client, chPtrBuffer, sizeof(chPtrBuffer));
			clock_gettime(CLOCK_MONOTONIC_RAW, &timeBegin);
		}
	}

}

static void childWork(CHPTR chPtrCommand) {
	
	FILEPTR filePtrTemp;
	FILEDESC fileDescServer2Client; 
	char path[PATH_MAX];
	char chPtrFunction[10];
	pid_t iCliChildPid, iCliChildsParentPid, iChildPid;
	int clientRecTime;
	char chPtrLogBuffer[LINE_MAX];
	STRTIMESPEC strTimeBegin, strTimeEnd;

	sscanf(chPtrCommand, CLIENTTOSERVERCOMMANDPATTERN, chPtrFunction, &clientRecTime, 
													   &iCliChildPid, &iCliChildsParentPid);
	clock_gettime(CLOCK_MONOTONIC_RAW, &clientConnected);
	sprintf(chPtrLogBuffer, "Client connected pid = %d, T(mili sec) = %d Function = %s",
			iCliChildPid, clientRecTime, chPtrFunction);
	appendLog(chPtrLogBuffer);
	memset(chPtrLogBuffer, '\0', LINE_MAX);

	switch (iChildPid = fork()) {
	case -1:
		perror("Error on fork() ");
		appendLog("Error on fork");
		finalize(3);
		break;
	case 0:
		break;
	default:
		return;
		break;
	}
	
	/* Creating the path of server's response FIFO */
	sprintf(path, SERVER2CLIENTFIFOPATTERN, iCliChildPid);

	/* Creating and opening a new FIFO for server respond */
	mkfifo(path, 0666); 
	fileDescServer2Client = open(path, O_WRONLY);
	
	strcat(path, "temp.txt");

	if ((filePtrTemp = fopen(path, "w")) == NULL ) {
		perror("Could not open temporary file.");
		finalize(EXITCODE_TEMPFILE);
	}

	clock_gettime(CLOCK_MONOTONIC_RAW, &strTimeBegin);

	function(fileDescServer2Client,chPtrFunction,clientRecTime);
	
	clock_gettime(CLOCK_MONOTONIC_RAW, &strTimeEnd);

	sprintf(chPtrLogBuffer, "Calculation completed for pid = %d in %ld msecs",
			iCliChildPid, timevalDiffInMicSec(&strTimeBegin, &strTimeEnd));
	appendLog(chPtrLogBuffer);


#ifndef NDEBUG
	sleep(1);
#endif
	char tempBuf[255];
	
	sprintf(tempBuf, "time dif:%ld nanoseconds",
			timevalDiffInNSec(&strTimeBegin, &strTimeEnd));
	
	write(fileDescServer2Client,tempBuf,sizeof(tempBuf));
	
	close(fileDescServer2Client);
	fclose(filePtrTemp);
	exit(EXIT_SUCCESS);
}

bool serverValidRequest(CHPTR request, long int requestTime){
   
 	if(!((strcmp(request,""))&&(requestTime>0)))
		return false;		
	return true;	
}


int main(int argc, CHPTR *argv) {
	char chArrBuff[PIPE_BUF];

	pidtParent = getpid();
		
	atexit(atExit);

	if (argc != 3) {
		fputs("Usage:[EXECUTABLE] <FUNCTION> <DIFFTIME NANOSECONDS> \n", stderr);
		exit(EXITCODE_USAGE);
	}
	
	chPtrMainFunction = argv[1];
	serverNanoSec = atol(argv[2]);
	
	if(!(serverValidRequest(chPtrMainFunction,serverNanoSec))){
		perror("Server arguments is not valid");
		finalize();
	}

	if (signal(SIGINT, signalHandler) == SIG_ERR ) {
		fputs("An error occurred while setting a signal handler.\n", stderr);
		appendLog("An error occurred while setting a signal handler.");
		finalize(EXITCODE_SIGNALHANDLING);
	}

	/* Signal handler for SIGCHILD */
	if (signal(SIGCHLD, catchChildProc) == SIG_ERR ) {
		fputs("An error occurred while setting a signal handler.\n", stderr);
		appendLog("An error occurred while setting a signal handler.");
		finalize(EXITCODE_SIGNALHANDLING);
	}

	if ((filePtrServerLog = fopen(TEMPSERVERLOGPATH, "w+")) == NULL ) {
		perror("Could not create temporary server log file");
		finalize(EXITCODE_LOGFILE);
	}
	
	appendLog("Server started");

	/* create the FIFO (named pipe) */
	mkfifo(chPTrC2SFifo, 0666);

	while (1) {
		/* open, read, and display the message from the FIFO */
		fdClient2Server = open(chPTrC2SFifo, O_RDONLY);

		if (read(fdClient2Server, chArrBuff, PIPE_BUF) > 1
				&& strlen(chArrBuff) > 5) {
			childWork(chArrBuff);
			/* clean buf from any data */
			memset(chArrBuff, 0, PIPE_BUF);
			close(fdClient2Server);
		}
	}

	unlink(chPTrC2SFifo);
	finalize(0);
	/* for hiding the warnings */
	kill(getpid(),15);
	return 0;
}
