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
 * 	luDecomposition.c
 */
 
//Libraries declaration
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

#include "myTypes.h"

#define MAX_SIZE_MATRIX 2

//message queues structs
struct my_msgbuf {
    long mtype;
    double myArray[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
    int numberOfMatrices;
};

struct my_msgbuf2 {
	long mtype;
	double myArray[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
	double lowerArray[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
	double upperArray[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
};
//global vars
double **lower,**upper,**matrix; 
struct my_msgbuf2 buf2;
int msqid2;
key_t key2;
// finalize for ctrl +c signal
void finalize(){
	
	int i_x;
	//memory free
	for(i_x=0;i_x<MAX_SIZE_MATRIX;++i_x){
				free(matrix[i_x]);
				free(lower[i_x]);
				free(upper[i_x]);
			}
			free(matrix);
			free(lower);
			free(upper);
	
	//close message queue 
	if (msgctl(msqid2, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }
		
	exit(1);
	
	
}
//signal handler for ctrl + c function
static void signalHandler() {
	fputs("Ctrl+C signal detected client is terminated\n",stderr);
	finalize();
}
//find lower and upper triangle matrices function
void findLowerAndUpperMatrix(double** dMatrix,double** dLowerMatrix,double** dUpperMatrix){
	
	int i_x,i_y,i_z,i_k;
    
	for(i_x=0;i_x<MAX_SIZE_MATRIX;i_x++)
		for(i_y=0;i_y<MAX_SIZE_MATRIX;i_y++)
			{
				if(i_x>i_y)
					 dUpperMatrix[i_x][i_y]=0.0;
				if(i_x<i_y)
					dLowerMatrix[i_x][i_y]=0.0;
				if(i_x==i_y)
					dLowerMatrix[i_x][i_y]=1.0;
			}

    for(i_x=0;i_x<MAX_SIZE_MATRIX;i_x++)
    {
        dUpperMatrix[i_x][i_x]=dMatrix[i_x][i_x];
            for(i_y=i_x+1;i_y<MAX_SIZE_MATRIX;i_y++)
            {
                dUpperMatrix[i_x][i_y]=dMatrix[i_x][i_y];
                dLowerMatrix[i_y][i_x]=dMatrix[i_y][i_x]/dUpperMatrix[i_x][i_x];
            }
             for(i_z=i_x+1;i_z<MAX_SIZE_MATRIX;i_z++)
                for(i_k=i_x+1;i_k<MAX_SIZE_MATRIX;i_k++)
                    dMatrix[i_z][i_k]=dMatrix[i_z][i_k]-(dLowerMatrix[i_z][i_x]*dUpperMatrix[i_x][i_k]);
    }
    
    
}
//printing matrix function
void printMatrix(double dMatrix[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX]){
	
	int i_x,i_y;
		
	printf("\n\n");
	for (i_x = 0; i_x < MAX_SIZE_MATRIX; i_x++) {
		for (i_y = 0; i_y < MAX_SIZE_MATRIX; i_y++) {
			printf("%.2f\t", dMatrix[i_x][i_y]);
		}
		printf("\n");
	}
}
//main function
int main(int argc,char* argv[]){
		
	
	double tempMatrix[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
	double dMatrix[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
	struct my_msgbuf buf;
    int msqid;
    int i_x,i_y,iCtr;
    key_t key;
	char c='.';
	int numOfMatrices = 200;

	//signal handling
	if (signal(SIGINT, signalHandler) == SIG_ERR ) {
		fputs("An error occurred while setting a signal handler.\n", stderr);
		finalize();
	}
	//open message queue for receive
    if ((key = ftok("matrixGenerator.c", 'B')) == -1) { 
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, 0644)) == -1) { 
        perror("msgget");
        exit(1);
    }
	//open message queue for send
    if ((key2 = ftok("luDecomposition.c", 'B')) == -1) {
        perror("ftok");
        exit(1);
    }

    if ((msqid2 = msgget(key2, 0644 | IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }
    
    buf2.mtype = 1;

	//memory allocation for matrices
	lower = calloc(MAX_SIZE_MATRIX, sizeof(double *));
	upper = calloc(MAX_SIZE_MATRIX, sizeof(double *));
	matrix = calloc(MAX_SIZE_MATRIX, sizeof(double *));
	for(i_x = 0; i_x < MAX_SIZE_MATRIX; i_x++) { 
	   lower[i_x] = calloc(MAX_SIZE_MATRIX, sizeof(double));
	   upper[i_x] = calloc(MAX_SIZE_MATRIX, sizeof(double));
	   matrix[i_x] = calloc(MAX_SIZE_MATRIX, sizeof(double));
	}
	//Recieving from matrixGenerator and sending
	//to InverseTaking module loop
	for(iCtr=0;iCtr<numOfMatrices;iCtr++) {
		//message recieving	
        if (msgrcv(msqid, &buf, sizeof(buf.numberOfMatrices)+
								sizeof(buf.myArray) , 0, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }	
        //assign matrices from queue
        for(i_x = 0; i_x< MAX_SIZE_MATRIX ;++i_x){
			for(i_y = 0 ;i_y <MAX_SIZE_MATRIX ;++i_y){
				dMatrix[i_x][i_y]=buf.myArray[i_x][i_y];
				tempMatrix[i_x][i_y]=dMatrix[i_x][i_y];
				matrix[i_x][i_y]=dMatrix[i_x][i_y];
			}
		}    
		//finds lower and upper triangle
		numOfMatrices = buf.numberOfMatrices;
		printf("\nMatrix num:%d\n",iCtr+1);
		printMatrix(tempMatrix);
		findLowerAndUpperMatrix(matrix,lower,upper);
		//assign other message queue for sending
		for(i_x = 0; i_x< MAX_SIZE_MATRIX ;++i_x){
			for(i_y = 0 ;i_y <MAX_SIZE_MATRIX ;++i_y){
				buf2.myArray[i_x][i_y]=tempMatrix[i_x][i_y];
				buf2.lowerArray[i_x][i_y]=lower[i_x][i_y];
				buf2.upperArray[i_x][i_y]=upper[i_x][i_y];
			}
		}
		//printing triangles
		printf("\nLower Matrix\n");
		printMatrix(buf2.lowerArray);
		printf("\nUpper Matrix\n");
		printMatrix(buf2.upperArray);
		
		//message send
		if (msgsnd(msqid2, &buf2, sizeof(buf2)-sizeof(long), 0) == -1)
            perror("msgsnd");

	
    }
	//wait for user
	do {
		c=getchar();
	}while (c == '.');
	
	//memory free
	for(i_x=0;i_x<MAX_SIZE_MATRIX;++i_x){
				free(matrix[i_x]);
				free(lower[i_x]);
				free(upper[i_x]);
			}
			free(matrix);
			free(lower);
			free(upper);
	
	//close message queue
	if (msgctl(msqid2, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }
    
	return 0;
}
