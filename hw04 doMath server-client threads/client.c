/*
 *	Ali Anil Apaydin 
 *  CSE244_HW04_091044042.c
 *  
 *  doMath Server and Integral/Derivate Client
 * 	with threads
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

#define LINENUM 3

/* Global variables declaration */
FILEPTR fPtrClient2Server;
CHPTR chPtrFunction;
int iClientRecTime;
pid_t pidtParentPid;
FILEPTR fPtrClientLog;
pthread_attr_t pthreadattr;

/* 
 * appendLog - prints the argument and date to
 * the client log file
 * params 
 * chPtrMessage - string that will be printed
 */ 
static void appendLog(CHPTR chPtrMessage) {
	time_t timetServerTime;
	time(&timetServerTime);
	fprintf(fPtrClientLog, "%-100s | %s", chPtrMessage,
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

	appendLog("Client is terminating.");
	if (!fPtrClientLog)
 		fclose(fPtrClientLog);
 
 	if (!fPtrClient2Server)
 		fclose(fPtrClient2Server);

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
	finalize(1);
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
	exit(iSignal);
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
	char chPtrS2CFifo[PATH_MAX];
	CHPTR chPtrC2SFifo = CLIENT2SERVERFIFO;
	FILEDESC fdServerRespondFile;
	FILEDESC fdServer2Client;
	STRTIMESPEC strTimespecTimeBegin, strTimespecTimeEnd;
	strCommand structCommand;

	sprintf(chPtrBuffer, "Function=%s Client Rec Time=%d. Client tid=%lu",chPtrFunction,iClientRecTime,
										pthread_self());
	appendLog(chPtrBuffer);

	memset(chPtrBuffer, '\0', PIPE_BUF);
	clock_gettime(CLOCK_MONOTONIC_RAW, &strTimespecTimeBegin);

	/* Creating a string to send server a command */
	structCommand.parentOfClientPid = getpid();
	structCommand.clientTid = pthread_self();
	strcpy(structCommand.chfunction,chPtrFunction);
	structCommand.iclientRecTime = iClientRecTime;

	/* Writing the command to well known FIFO*/
	if (!(fPtrClient2Server = fopen(chPtrC2SFifo, "wb"))) {
 		perror("Could not connect to server");
 		finalize();
 	}
 	fwrite(&structCommand, sizeof(strCommand), 1, fPtrClient2Server);
	memset(chPtrBuffer, '\0', PIPE_BUF);

	/* Closing the common FIFO */
	fclose(fPtrClient2Server);

	/* Creating the path of server's response FIFO */
	sprintf(chPtrS2CFifo, SERVER2CLIENTFIFOPATTERN, structCommand.clientTid);

	/* Creating and opening a new FIFO for server respond */
	mkfifo(chPtrS2CFifo, 0666); 
	fdServer2Client = open(chPtrS2CFifo, O_RDONLY);


	/* Opening file for server respond */
	sprintf(chPtrBuffer, FUNCTIONRESPONDPATTERN, chPtrFunction,iClientRecTime,
						pthread_self());
	if ((fdServerRespondFile = open(chPtrBuffer, O_WRONLY | O_CREAT | O_TRUNC,
			0666)) == -1) {
		perror("Could not create temp file. Process terminating");
		exit(EXITCODE_TEMPFILE);
	}

	/* Waiting server to write directory list to FIFO and read it */
	while (read(fdServer2Client, chPtrBuffer, sizeof(chPtrBuffer)))
		appendLog(chPtrBuffer);

	clock_gettime(CLOCK_MONOTONIC_RAW, &strTimespecTimeEnd);

#ifndef NDEBUG
	sleep(1);
#endif

	close(fdServerRespondFile);

	sprintf(chPtrBuffer, "%s '%s' '%d'. tid %lu in %lu nano seconds.", "done",
			chPtrFunction, iClientRecTime,pthread_self(),
			timevalDiffInNSec(&strTimespecTimeBegin, &strTimespecTimeEnd));
	
	appendLog(chPtrBuffer);

	memset(chPtrBuffer, '\0', PIPE_BUF);

	/* Closing the FIFO between server's child and client's child. */
	close(fdServer2Client);
	unlink(chPtrS2CFifo);
	pthread_exit(EXIT_SUCCESS);

}
/*
 *  clientValidRequest - controls client request arguments
 * 
 *  params
 *  chPtrRequest - char pointer keeps client's function type
 *  dRequestTime - double keeps client's time in miliseconds
 */
bool clientValidRequest(CHPTR chPtrRequest, double dRequestTime){
   
 	if(!(((!strcmp(chPtrRequest,"integral"))||(!strcmp(chPtrRequest,"derivate")))
		&& dRequestTime > 0))
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

	if (argc != 3) {
		fputs("Usage:[EXECUTABLE] <INTEGRAL - DERIVATE> <TIME MILISECONDS> \n", 
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

	chPtrFunction = argv[1];
	iClientRecTime = atof(argv[2]);
	
	if(!(clientValidRequest(chPtrFunction,iClientRecTime))){
		perror("Client arguments is not valid");
		finalize();
	}

	function();

	finalize();
	
	return 0; /* Prevent warnings */
}
