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
 * 	verifier.c
 */
 
//libraries declaration
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#include "myTypes.h" 

//multiplication for Verifierer function
//multiply two matrices
void multiplicationForVerifier(double** matrix ,double** firstMatrix , double** secondMatrix,int iMatrixSize){
	
	int i,j,e;
	double sum;
	
	for (i=0;i<iMatrixSize;i++)
        for (j=0;j<iMatrixSize;j++) {
            sum=0;
            for (e=0;e<iMatrixSize;e++)
                sum+=firstMatrix[i][e]*secondMatrix[e][j];
            matrix[i][j]=sum;
        }
} 
