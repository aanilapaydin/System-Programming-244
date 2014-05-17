/*
 *	Ali Anil Apaydin 
 *  CSE244_MID_091044042.c
 *  
 *  doMath Server and Integral/Derivate Client
 *  
 *  commonHeader.h
 */

#ifndef COMMONHEADER_H_
#define COMMONHEADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

#ifndef TYPES_H_
#include "types.h"
#endif

#define CLIENT2SERVERFIFO "/tmp/server_to_client_fifo"
#define SERVER2CLIENTFIFOPATTERN "/tmp/doMath.%d"
#define FINDRESPONDPATTERN "/tmp/doMath.%s.%d.%d.txt"
#define CLIENTTOSERVERCOMMANDPATTERN "%s %d %d %d"

#define TEMPSERVERLOGPATH "/tmp/doMathLog.txt"
#define TEMPCLIENTLOGPATH "/tmp/doMath.clientLog.%d.txt"

#define EXITCODE_SIGNALHANDLING 5
#define EXITCODE_LOGFILE 6
#define EXITCODE_TEMPFILE 9
#define EXITCODE_USAGE 2
#define EXITCODE_FIFOOPEN 2
#define EXITCODE_FORKERROR 7
#define SERVERDIE 11
#define ATEXIT 12

#define MAXCLIENTREQUEST 1024

#define MILLION 1E6
#define BILLION  1E9

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef LINE_MAX
#define LINE_MAX 2048
#endif

#ifndef FILENAME_MAX
#define FILENAME_MAX 4096
#endif

#ifndef PIPE_BUF
#define PIPE_BUF 4096
#endif

#define SIGSERVERDIE SIGUSR1

typedef int bool;
enum { false, true };

long timevalDiffInNSec(STRTIMESPECPTR timeBegin,
		STRTIMESPECPTR timeEnd) {
	if((timeEnd->tv_nsec - timeBegin->tv_nsec) < 0)
		return ((BILLION+timeEnd->tv_nsec-timeBegin->tv_nsec) +
						(timeEnd->tv_sec-timeBegin->tv_sec-1)*BILLION);
	else
		return ((timeEnd->tv_nsec-timeBegin->tv_nsec) +
						(timeEnd->tv_sec-timeBegin->tv_sec)*BILLION);
}

long timevalDiffInMicSec(STRTIMESPECPTR timeBegin,
		STRTIMESPECPTR timeEnd) {
	return (timevalDiffInNSec(timeBegin,timeEnd)/1000.0);
}

pid_t r_wait(int *stat_loc) {
	pid_t retval;
	while (((retval = wait(stat_loc)) == -1) && (errno == EINTR))
		;
	return retval;
}


#endif /* COMMONHEADER_H_ */
