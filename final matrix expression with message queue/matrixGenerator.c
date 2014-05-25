/*
 *
 *  Created on: May 21,2014
 *  Author: Ali Anil Apaydin
 *  
 *  CSE244_FINAL_091044042
 *  
 *  Description:Matrix inverting,decomposition,
 *  	 	verification,multiplication
 *  
 *
 * 	matrixGenerator.c
 */

//Libraries Declaration
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <signal.h>

#include "myTypes.h"

#define MAX_SIZE_MATRIX 2 

//Message queue struct
struct my_msgbuf{
	
	long mtype;
	double myArray[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
	int numberOfMatrices;
	
};
//Global vars
struct my_msgbuf buf;
int msqid;
key_t key;


//Finalize function for Ctrl+C signal
void finalize(){
	//close message queue
	if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }
		
	exit(1);
	
	
}
//Signal handler function for ctrl + c signal
static void signalHandler() {
	fputs("Ctrl+C signal detected client is terminated\n",stderr);
	finalize();
}
//Random matrix generator
void matrixGenerateRandVal(double dMatrix[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX],
						int iMatrixSize){

	int i_x,i_y;

	
	for(i_x = 0 ; i_x<iMatrixSize ; ++i_x){
		for(i_y = 0 ; i_y<iMatrixSize ; ++i_y){
			dMatrix[i_x][i_y]=((rand()%20-10+((rand()%10)/10.0)));
		}	
	}

}
//Argument validation checking function
bool argumentCheck(CHPTR matrixSize,CHPTR numOfMatrices){
	
	if(atoi(matrixSize)>0 && atoi(matrixSize) <= MAX_SIZE_MATRIX && 
		atoi(numOfMatrices)>0)
		return true;
	
	return false;
}
//Printing usage function
void usage(){
	fprintf(stderr,"[EXECUTABLE] [N (NxN Matrix SIZE)] [K (NUMBER OF MATRICES)]\n");
}
//print matrix function
void printMatrix(double dMatrix[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX],int iMatrixSize){
	
	int i_x,i_y;
		
	printf("\n\n");
	for (i_x = 0; i_x < iMatrixSize; i_x++) {
		for (i_y = 0; i_y < iMatrixSize; i_y++) {
			printf("%.2f\t", dMatrix[i_x][i_y]);
		}
		printf("\n");
	}
}
//main function
int main(int argc,char* argv[]){

	int iMatrixSize,numOfMatrices;
	int i_x,i_y,iCtr;
	double dMatrix[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
	char c='.';
	srand(time(NULL));


	if (signal(SIGINT, signalHandler) == SIG_ERR ) {
		fputs("An error occurred while setting a signal handler.\n", stderr);
		finalize();
	}

	//Argument check
	if(argc!=3){
		//usage();
		exit(1);
	}

	if(!argumentCheck(argv[1],argv[2])){
		fprintf(stderr,"Arguments are not valid\n");
		exit(1);
	}
	
	iMatrixSize=atoi(argv[1]);
	numOfMatrices = atoi(argv[2]);
	//open message queue
    if ((key = ftok("matrixGenerator.c", 'B')) == -1) {
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, 0644 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }
    
    buf.mtype=1;   
    
    //generating matrices and sending them to lu module loop
    for(iCtr=0;iCtr<numOfMatrices;++iCtr){
		//matrix generation
		matrixGenerateRandVal(dMatrix,iMatrixSize);
		//assign the message queue struct
		for(i_x=0;i_x<iMatrixSize;i_x++){
			for(i_y=0;i_y<iMatrixSize;i_y++){
				buf.myArray[i_x][i_y]=dMatrix[i_x][i_y];
			}
		}
		buf.numberOfMatrices=numOfMatrices;
		printMatrix(dMatrix,iMatrixSize);
		//message sending
		if (msgsnd(msqid, &buf, sizeof(buf.myArray)+
								sizeof(buf.numberOfMatrices), 0) == -1)
            perror("msgsnd");
		
	}
	//waiting for user
	do {
		c=getchar();
	}while (c == '.');
	//close message queue
	if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }
    
	return 0;
}
