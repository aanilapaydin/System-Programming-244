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
 * 	inverseTaking.c
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
#include "verifier.c"

#define MAX_SIZE_MATRIX 2

//Global vars
double dLowerDeterminant;
double dUpperDeterminant;
double** dInverseMatrix; 
double** dInverseLowerMatrix; 
double** dInverseUpperMatrix; 
double** dMatrix;
double** dLowerMatrix;
double** dUpperMatrix;
double** dIMatrix;
double** tempMatrix;
int iOrderMatrix = MAX_SIZE_MATRIX; 
int iMatrixSize = MAX_SIZE_MATRIX;

//message queue struct
struct my_msgbuf {
	long mtype;
	double myArray[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
	double lowerArray[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
	double upperArray[MAX_SIZE_MATRIX][MAX_SIZE_MATRIX];
};
//finalize function for ctrl + c signal
void finalize(){
	
	int i_x;
	//memory free
	for(i_x=0;i_x<MAX_SIZE_MATRIX;++i_x){
			free(dMatrix[i_x]);
			free(dLowerMatrix[i_x]);
			free(dUpperMatrix[i_x]);
			free(dInverseMatrix[i_x]);
			free(dInverseLowerMatrix[i_x]);
			free(dInverseUpperMatrix[i_x]);
			free(dIMatrix[i_x]);
			free(tempMatrix[i_x]);
		}
		free(dMatrix);
		free(dLowerMatrix);
		free(dUpperMatrix);
		free(dInverseMatrix);
		free(dInverseLowerMatrix);
		free(dInverseUpperMatrix);
		free(dIMatrix);
		free(tempMatrix);
		
	exit(1);
	
	
}
//signal handler for ctrl +c signal
static void signalHandler() {
	fputs("Ctrl+C signal detected client is terminated\n",stderr);
	finalize();
}
//find determinant function
double determinant(double **matrix,int iOrder){
	
	 double s = 1, det = 0;
	 double **b;
	 int i,j,m,n,c;
	 
	 b = calloc(iMatrixSize, sizeof(double *));
	 for(i = 0; i < iMatrixSize; i++) { 
	    b[i] = calloc(iMatrixSize, sizeof(double));
	 }
	 
	 if (iOrder == 1){
	  for(i=0;i<iMatrixSize;++i){
		free(b[i]);
	  }
	  free(b);
	  return (matrix[0][0]);
	 }
	 else {
		det = 0;
		for (c = 0; c < iOrder; c++) {
			m = 0;
			n = 0;
			for (i = 0; i< iOrder ; i++) {				
				for (j = 0 ; j <iOrder ; j++){
					b[i][j] = 0;
					if (i != 0 && j != c) {
						b[m][n] = matrix[i][j];
						if (n < (iOrder - 2))
							n++;
						else {
							n = 0;
							m++;
						}
					}
				}
			} 
			det = det + s * (matrix[0][c] * determinant(b,iOrder - 1));
			s = -1 * s;
		}
	 }
	 
	 for(i=0;i<iMatrixSize;++i){
		free(b[i]);
	 }
	 free(b);
	 
	 return det;
}
//find tranpoze matrix function
void trans(double** inv,double** num, double** fac, float r)
{
	int i, j;
	double** b; 
	double d;
	
	b = calloc(iMatrixSize, sizeof(double *));
	for(i = 0; i < iMatrixSize; i++) { 
	   b[i] = calloc(iMatrixSize, sizeof(double));
	}
	
	for (i = 0; i < r; i++) {
		for (j = 0; j < r; j++) {
			b[i][j] = fac[j][i];
		}
	}
	
	d = determinant(num, r);

	for (i = 0; i < r; i++) {
		for (j = 0; j < r; j++) {
			inv[i][j] = b[i][j] / d;
		}
	}

	for(i=0;i<iMatrixSize;++i){
		free(b[i]);
	}
	free(b);
}
//find cofactors functions
void cofactors(double** inv,double** num, float f) {
	
	double** b;
	double** fac;
	int p, q, m, n, i, j;
 
	b = calloc(iMatrixSize, sizeof(double *));
	fac = calloc(iMatrixSize, sizeof(double *));
	for(i = 0; i < iMatrixSize; i++) { 
	   b[i] = calloc(iMatrixSize, sizeof(double));
	   fac[i] = calloc(iMatrixSize, sizeof(double));
	}

	for (q = 0; q < f; q++) {
		for (p = 0; p < f; p++) {
			m = 0;
			n = 0;
			for (i = 0; i < f; i++) {
				for (j = 0; j < f; j++) {
					b[i][j] = 0;
					if (i != q && j != p) {
						b[m][n] = num[i][j];
						if (n < (f - 2))
							n++;
						else {
							n = 0;
							m++;
						}
					}
				}
			}
			fac[q][p] = pow(-1, q + p) * determinant(b, f - 1);
		}
	}
	trans(inv,num,fac,f);
	for(i=0;i<iMatrixSize;++i){
		free(b[i]);
		free(fac[i]);
	}
	free(b);
	free(fac);
}
//printing matrix function
void printMatrix(double** dMatrix){
	
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
	
	//Variable Definition
	int i_x,i_y;
	struct my_msgbuf buf;
    int msqid;
    key_t key;
	
	iOrderMatrix=MAX_SIZE_MATRIX;
	
	//open message queue for receive
    if ((key = ftok("luDecomposition.c", 'B')) == -1) { 
        perror("ftok");
        exit(1);
    }

    if ((msqid = msgget(key, 0644)) == -1) { 
        perror("msgget");
        exit(1);
    }
	//ctrl + c signal handling 
	if (signal(SIGINT, signalHandler) == SIG_ERR ) {
		fputs("An error occurred while setting a signal handler.\n", stderr);
		finalize();
	}
	
	//memory allocation for matrices
	dLowerMatrix = calloc(iMatrixSize, sizeof(double *));
	dUpperMatrix = (double**)malloc(iMatrixSize*sizeof(double));
	tempMatrix = (double**)malloc(iMatrixSize*sizeof(double));
	dMatrix = calloc(iMatrixSize, sizeof(double *));
	for(i_x = 0; i_x < MAX_SIZE_MATRIX; i_x++) { 
	   dUpperMatrix[i_x] = (double*)malloc(iMatrixSize*sizeof(double));
	   dLowerMatrix[i_x] = calloc(iMatrixSize, sizeof(double));
	   tempMatrix[i_x] = (double*)malloc(iMatrixSize*sizeof(double));
	   dMatrix[i_x] = calloc(iMatrixSize, sizeof(double));
	}
	
	dInverseLowerMatrix = (double**)malloc(iMatrixSize*sizeof(double));
	dInverseUpperMatrix = (double**)malloc(iMatrixSize*sizeof(double));
	for(i_x = 0; i_x < MAX_SIZE_MATRIX; i_x++) { 
	   dInverseLowerMatrix[i_x] = (double*)malloc(iMatrixSize*sizeof(double));
	   dInverseUpperMatrix[i_x] = (double*)malloc(iMatrixSize*sizeof(double));
	}

	dInverseMatrix = (double**)malloc(iMatrixSize*sizeof(double));
	for(i_x = 0; i_x < MAX_SIZE_MATRIX; i_x++) { 
	   dInverseMatrix[i_x] = (double*)malloc(iMatrixSize*sizeof(double));
	}
	dIMatrix = (double**)malloc(iMatrixSize*sizeof(double));
	for(i_x = 0; i_x < iMatrixSize; i_x++) { 
	   dIMatrix[i_x] = (double*)malloc(iMatrixSize*sizeof(double));
	}

	//infinite loop for final recieving only exit ctrl + c
	for(;;) {
		//receive matrices
        if (msgrcv(msqid, &buf, sizeof(buf)-sizeof(long), 0, 0) == -1) {
            perror("msgrcv");
            exit(1);
        }
		//assign matrices
        for(i_x = 0; i_x< MAX_SIZE_MATRIX ;++i_x){
			for(i_y = 0 ;i_y <MAX_SIZE_MATRIX ;++i_y){
			   dUpperMatrix[i_x][i_y] = buf.upperArray[i_x][i_y];
			   dLowerMatrix[i_x][i_y] = buf.lowerArray[i_x][i_y];
			   tempMatrix[i_x][i_y] = buf.myArray[i_x][i_y];
			   dMatrix[i_x][i_y] = tempMatrix[i_x][i_y];
			}
		}
		//find lower determinant and inverse lower
		dLowerDeterminant = determinant(dLowerMatrix,iOrderMatrix);
		printf("\nTHE LOWER DETERMINANT IS=%2f\n", dLowerDeterminant);
		if (dLowerDeterminant == 0)
			printf("\nLOWER MATRIX IS NOT INVERSIBLE\n");
		else
			cofactors(dInverseLowerMatrix,dLowerMatrix,iOrderMatrix);
		//find upper determinant and inverse upper
		dUpperDeterminant = determinant(dUpperMatrix,iOrderMatrix);
		printf("\nTHE UPPER DETERMINANT IS=%2f\n", dUpperDeterminant);
		if (dUpperDeterminant == 0)
			printf("\nUPPER MATRIX IS NOT INVERSIBLE\n");
		else
			cofactors(dInverseUpperMatrix,dUpperMatrix,iOrderMatrix);
		
		//print inverse lower and upper
		printf("\nTHE INVERSE OF THE LOWER MATRIX:\n");
		printMatrix(dInverseLowerMatrix);
		printf("\nTHE INVERSE OF THE UPPER MATRIX:\n");
		printMatrix(dInverseUpperMatrix);
		
		//multiplication for U^-1 * L^-1 for find inverse A
		multiplicationForVerifier(dInverseMatrix,dInverseUpperMatrix,dInverseLowerMatrix,MAX_SIZE_MATRIX);

		//printing inverse A
		printf("\nTHE INVERSE OF THE MATRIX BY (U^-1)*(L^-1):\n");
		printMatrix(dInverseMatrix);
		
		//VERIFIERER 
		//multiplication for A^-1 * A = I
		multiplicationForVerifier(dIMatrix,dMatrix,dInverseMatrix,MAX_SIZE_MATRIX);
		//printing I matrix
		printf("\nTHE I MATRIX BY (A)*(A^-1):\n");
		printMatrix(dIMatrix);

	}
	
	return 0;
}
