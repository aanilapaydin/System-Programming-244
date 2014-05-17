/*
 *	Ali Anil Apaydin 
 *  CSE244_MID_091044042.c
 *  
 *  doMath Server and Integral/Derivate Client
 *  
 *  client.c
 */

#ifndef COMMONHEADER_H
#include "commonHeader.h"
#endif

#ifndef TYPES_H
#include "types.h"
#endif

#include <termios.h>
#include <sys/ioctl.h>
#include <termios.h>

#define LINENUM 3


FILEDESC fdClient2Server;
CHPTR chPtrFunction;
int clientRecTime;
pid_t pidtParentPid;
FILEPTR filePtrClientLog;
struct termios defalutTerminalAttributes;


static void appendLog(CHPTR chPtrMessage) {
	time_t timetServerTime;
	time(&timetServerTime);
	fprintf(filePtrClientLog, "%-80s | %s", chPtrMessage,
			ctime(&timetServerTime));
	fflush(filePtrClientLog);
}

void atExit(void) {
	char chPtrCwd[PATH_MAX], chPtrCommand[LINE_MAX], chPtrLogPath[PATH_MAX];

	if (getpid() != pidtParentPid)
		return;

	while (r_wait(NULL ) > 0)
		; /* wait for all of your children */


	appendLog("Client is terminating.");
	fclose(filePtrClientLog);
	close(fdClient2Server);

	sprintf(chPtrLogPath, TEMPCLIENTLOGPATH, getpid());
	getcwd(chPtrCwd, PATH_MAX);

	sprintf(chPtrCommand, "mv %s %s/client.%d.log", chPtrLogPath, chPtrCwd,
			getpid());
	system(chPtrCommand);

	return;
}

void finalize(int iExitCode) {
	
	atExit();
	exit(iExitCode);
}

static void catchChildProc(int iSignal) {
	wait3(NULL, WNOHANG, NULL );
}

static void signalHandler() {
	
	if(getpid()!=pidtParentPid){
		kill(getpid(),15);
	}
	appendLog("Ctrl+C signal detected client is terminated\n");
	fputs("Ctrl+C signal detected client is terminated\n",stderr);
	finalize(1);
}

static void serverDie(int iSignal) {

	if (getpid() != pidtParentPid) {
		kill(getppid(), SIGSERVERDIE);
		exit(SERVERDIE);
	}

	appendLog("Server has died. Client is terminating.");
	exit(SERVERDIE);
}

void function() {
	
	char chPtrBuffer[PIPE_BUF];
	char chPtrS2CFifo[PATH_MAX];
	FILEDESC fdServerRespondFile;
	FILEDESC fdServer2Client;
	pid_t pidtChildPid;
	STRTIMESPEC strTimeBegin, strTimeEnd;

	switch (pidtChildPid = fork()) {
	case -1:
		perror("Could not fork");
		finalize(EXITCODE_FORKERROR);
		break;
	case 0:
		break;
	default:
		return;
	}

	sprintf(chPtrBuffer, "%s %d. pid %d",chPtrFunction,clientRecTime,getpid());
	appendLog(chPtrBuffer);

	memset(chPtrBuffer, '\0', PIPE_BUF);
	clock_gettime(CLOCK_MONOTONIC_RAW, &strTimeBegin);

	/* Creating a string to send server a command */
	sprintf(chPtrBuffer, CLIENTTOSERVERCOMMANDPATTERN, chPtrFunction,clientRecTime,
													   getpid(),getppid());

	/* Writing the command to well known FIFO */
	write(fdClient2Server, chPtrBuffer, sizeof(chPtrBuffer));
	memset(chPtrBuffer, '\0', PIPE_BUF);

	/* Closing the common FIFO */
	close(fdClient2Server);

	/* Creating the path of server's response FIFO */
	sprintf(chPtrS2CFifo, SERVER2CLIENTFIFOPATTERN, getpid());

	/* Creating and opening a new FIFO for server respond */
	mkfifo(chPtrS2CFifo, 0666); 

	fdServer2Client = open(chPtrS2CFifo, O_RDONLY);


	/* Opening file for server respond */
	sprintf(chPtrBuffer, FINDRESPONDPATTERN, chPtrFunction,clientRecTime,getpid());
	if ((fdServerRespondFile = open(chPtrBuffer, O_WRONLY | O_CREAT | O_TRUNC,
			0666)) == -1) {
		perror("Could not create temp file. Process terminating");
		exit(EXITCODE_TEMPFILE);
	}

	/* Waiting server to write directory list to FIFO and read it */
	while (read(fdServer2Client, chPtrBuffer, sizeof(chPtrBuffer)))
		printf("%s\n",chPtrBuffer);

	clock_gettime(CLOCK_MONOTONIC_RAW, &strTimeEnd);

#ifndef NDEBUG
	sleep(1);
#endif

	close(fdServerRespondFile);

	sprintf(chPtrBuffer, "%s '%s' '%d'. pid %d in %ld nano seconds.", "done",
			chPtrFunction, clientRecTime,getpid(),
			timevalDiffInNSec(&strTimeBegin, &strTimeEnd));
	appendLog(chPtrBuffer);

	memset(chPtrBuffer, '\0', PIPE_BUF);

	/* Closing the FIFO between server's child and client's child. */
	close(fdServer2Client);
	unlink(chPtrS2CFifo);

}

bool clientValidRequest(CHPTR request, double requestTime){
   
 	if(!(((!strcmp(request,"integral"))||(!strcmp(request,"derivate")))
		&& requestTime > 0))
		return false;		
	return true;	
}

int main(int argc, CHPTR *argv) {
	
	CHPTR chPTrC2SFifo = CLIENT2SERVERFIFO;
	char chArrBuff[PATH_MAX];
	pidtParentPid = getpid();

	if (argc != 3) {
		fputs("Usage:[EXECUTABLE] <INTEGRAL - DERIVATE> <TIME MILISECONDS> \n", stderr);
		exit(EXITCODE_USAGE);
	}
	
	/* Signal handler for ctrl+C */
	if (signal(SIGINT, signalHandler) == SIG_ERR ) {
		fputs("An error occurred while setting a signal handler.\n", stderr);
		appendLog("An error occurred while setting a signal handler.");
		finalize(EXITCODE_SIGNALHANDLING);
	}
	
	/* Signal handler for SIGCHILD */
	if (signal(SIGCHLD, catchChildProc) == SIG_ERR ) {
		perror("An error occurred while setting a signal handler.\n");
		finalize(EXITCODE_SIGNALHANDLING);
	}

	/* Signal handler for SERVERDIE */
	if (signal(SERVERDIE, serverDie) == SIG_ERR ) {
		perror("Could not create temporary client log file");
		finalize(SERVERDIE);
	}
	
	sprintf(chArrBuff, TEMPCLIENTLOGPATH, getpid());

	if ((filePtrClientLog = fopen(chArrBuff, "w+")) == NULL ) {
		perror("Could not create temporary client log file");
		finalize(EXITCODE_LOGFILE);
	}
	memset(chArrBuff, 0, PIPE_BUF);

	appendLog("Client Started.");

	chPtrFunction = argv[1];
	clientRecTime = atof(argv[2]);
	
	if(!(clientValidRequest(chPtrFunction,clientRecTime))){
		perror("Client arguments is not valid");
		finalize(2);
	}

	/* opening well-known FIFO */
	if ((fdClient2Server = open(chPTrC2SFifo, O_WRONLY)) == -1) {
		perror("Could not connect to server");
		finalize(3);
	}

	function();

	finalize(EXIT_SUCCESS);
	
	return 0; /* Prevent warnings */
}
