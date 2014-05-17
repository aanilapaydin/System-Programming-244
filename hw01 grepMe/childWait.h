/*
*	Ali Anil Apaydin
*	091044042 
*
*	CSE244 - System Programming 
*	Homework 01
*
*	Grep function with fork
*	childWait.h
*/

#include <errno.h>
#include <sys/wait.h>

pid_t childWait(int *stat) {
   int retval;

   while (((retval = wait(stat)) == -1) && (errno == EINTR)) ;
   return retval;
}
