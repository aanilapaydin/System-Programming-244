#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "mathFunctions.h"


int isDigit(char digit)
{
		if(digit >= 48 && digit <= 57)
			return 1;
		else
			return 0;
}

int isNumber()
{
	int i;
	int len = globNextIter - globPrevIter;
	char token[len];
	int digitNum = 0;
	int dotChecking = 0;

	strncpy(token,&globArgv[globPrevIter],len);
	token[len] = '\0';

	for(i=0; i<len; ++i)
	{
		if(!((token[i] >= 48 && token[i] <= 57) ||
			(token[i] == 46 && dotChecking == 0 && digitNum > 0)))
		{
			if(digitNum > 0)
			{
				return -1;
			}
			else
			{
				return -2;
			}
		}
		
		if(token[i] == 46)
			dotChecking = 1;
			
		++digitNum; 
	}

	return 1;
}

int isOperator()
{
	int i;
	int len;
	char token[10];

	len = globNextIter - globPrevIter;
	strncpy(token,&globArgv[globPrevIter],len);
	token[len] = '\0';
	
	for(i=0; i<LEN_OF_OP; ++i)
	{
		if(strncmp(token,OPERATORS[i],len) == 0)
		{
			if(strcmp(token,"(") == 0)
			{
				paranIndex = globNextIter - 1;
				++closeParanFlag;
			}
			if(strcmp(token,")") == 0)
			{
				paranIndex = globNextIter - 1;
				--closeParanFlag;
			}
				
			return PRECEDENCE[i];
		}
	}

	return 0;
}

int isParam()
{
	int len = globNextIter - globPrevIter;
	char token[len];

	strncpy(token,&globArgv[globPrevIter],len);
	token[len] = '\0';
	if(strncmp(token,"t",len) == 0)
	{
		return 1;
	}
	return 0;
}

int hasMoreToken()
{
	int tempIter;

	for(tempIter=globPrevIter; tempIter<globLenOfFunc; ++tempIter)
	{
		if(globArgv[tempIter] == ' ')
		{
			globNextIter = tempIter;
			return tempIter;
		}
	}	

	return -1;
}

void stringBuilder(char *newString)
{
	if(sizePostFix < globLenOfFunc - 1)
	{
		strcpy(postfix[sizePostFix++],newString);
		strcat(postfix[sizePostFix++]," ");
	}
}

void pushOp()
{
	int len = globNextIter - globPrevIter;
	if(sizeOpStack < globLenOfFunc - 1)
	{
		strncpy(opStack[sizeOpStack++], &globArgv[globPrevIter], len);
		strcat(opStack[sizeOpStack++]," ");
	}
}

void push(double newValue)
{
	resultOfPostfix[sizeOfResultStack++] = newValue;
}

void popOp()
{
	if(sizeOpStack > 1)
	{
		sizeOpStack -= 2;
		strcpy(topOfStack,opStack[sizeOpStack]);
	}
}

double pop()
{
	return resultOfPostfix[--sizeOfResultStack];
}

void peekOp()
{
	if(sizeOpStack > 1)
		strcpy(topOfStack,opStack[(sizeOpStack-2)]);
}

double peek()
{
	return resultOfPostfix[sizeOfResultStack - 1];
}

int getPrecedence( char *newOperator)
{
	int i=0;
	for(i=0; i<LEN_OF_OP; ++i)
	{
		if(strcmp(OPERATORS[i],newOperator) == 0)
		{
			return PRECEDENCE[i];
		}
	}
	
	return -2;
}

int getIndexOfOp( char *newOperator)
{
	int i=0;
	for(i=0; i<LEN_OF_OP; ++i)
	{
		if(strcmp(OPERATORS[i],newOperator) == 0)
		{
			return i;
		}
	}
	
	return -2;
}

void processOperator()
{
	int len = globNextIter - globPrevIter;
	char op[5];
	
	strncpy(op,&globArgv[globPrevIter],len);
	op[len] = '\0';
	
	if(sizeOpStack == 0 || strcmp(op,"(") == 0)
	{
		pushOp();
	}
	else
	{
		peekOp();
		if(getPrecedence(op) > getPrecedence(topOfStack))
		{
			pushOp();
		}
		else
		{
			while(sizeOpStack != 0 && 
				  getPrecedence(op) <= getPrecedence(topOfStack))
			{
				popOp();
				if(strcmp(topOfStack,"(") == 0)
				{
					// Matching '(' popped - exit loop
					break;
				}
				stringBuilder(topOfStack);
				if(sizeOpStack != 0)
				{
					// Reset topOfStack
					peekOp();
				}
			}
			if(strcmp(op,")") != 0)
				pushOp();
		}
	}
}

int convertToPostfix()
{
	
	
	char tempString[100];
	int tempIsOperator, tempIsNumber, tempIsParam,i;
	int len = 0;
	while(hasMoreToken() > 0)
	{
		tempIsOperator = isOperator() ;
		tempIsParam    = isParam();
		tempIsNumber   = isNumber();
		len = globNextIter - globPrevIter;
		
		if(tempIsNumber == 1)
		{
			strncpy(tempString,	&globArgv[globPrevIter],len);
			tempString[len] = '\0';
			stringBuilder(tempString);
		}
		else if(tempIsParam == 1)
		{
			strncpy(tempString,	&globArgv[globPrevIter], len);
			tempString[len] = '\0';
			stringBuilder(tempString);
			
		}
		else if(tempIsOperator != 0)
		{
			processOperator();
			if(closeParanFlag < 0)
			{
				printf("Missing Open Paranteshes Index of %d \n",
						paranIndex);
				return -1;
			}
		}
		else if(tempIsNumber == -1)
		{
			printf("Illegal Number Index of %d\n",globNextIter - 1);
			return -1;
		}
		else if(tempIsNumber == -2)
		{
			printf("Illegal Parameter Index of %d\n",globPrevIter);
			return -1;
		}
		else
		{
			printf("There is an error\n");
			return -1;
		}
		
		// checking whether over the array size
		if(globNextIter < globLenOfFunc - 1)
			globPrevIter = globNextIter + 1;
		else
		{
			globPrevIter = globNextIter;
			break;
		}
	}
	
	while(sizeOpStack != 0)
	{
		popOp();
		
		if(closeParanFlag < 0)
		{
			printf("Missing Open Paranteshes Index of %d \n",
						paranIndex);
			return -1;
		}
		
		if (strcmp(topOfStack,"(") == 0)
		{
			printf("Missing Close Paranteshes Index of %d \n",
						paranIndex);
			return -1;
		}
		stringBuilder(topOfStack);
	}
	
	for(i=0; i<sizePostFix;++i)
	{
		printf("%s",postfix[i]);
	}
	
	printf("\n");

return 1;
}

void reset()
{
	int i,j;
	for(i=0; i<OPROW; ++i )
	{
		resultOfPostfix[i] = '\0';
		for(j=0; j<OPCOL; ++j)
		{
			opStack[i][j] = '\0';
		}
	}
	
	for(i=0; i<sizeOfResultStack; ++i)
	{
		resultOfPostfix[i] = '\0';
	}
	
	sizeOpStack = 0;
	sizeOfResultStack = 0;
	globNextIter = 0;
	globPrevIter = 0;
}

long double postfixCalculator(char* chrPtrFunction)
{
	int i,len = strlen(globArgv); 
	char temp[len];
	long timedifFirst = 0, timedifSecond = 0;
	long double resultOfGivenFunction = 0.0;


	for(i=0; i<len; ++i)
	{
		temp[i] = '\0';
	}

	for(i=0; i<sizePostFix; ++i)
	{
		strcat(temp,postfix[i]);
	}
	reset();
	globArgv = temp;
	globLenOfFunc = strlen(globArgv);
			
	   timedifFirst = getNanoSec();
	   
	   if(!strcmp(chrPtrFunction,"integral")){
			resultOfGivenFunction = takeIntegration(currentParam);
	   }
	   else{
			resultOfGivenFunction = takeDerivation(currentParam);
	   
		}
		if(sizeOfResultStack != 0)
		{
			printf("Illegal Input\n");
			return -1;
		}
		else
		{
			currentParam += pendingTime;
			timedifSecond = getNanoSec();
			if((timedifSecond - timedifFirst) <= pendingTime)
			{	
			
				do{
					timedifSecond = getNanoSec();
				}while((timedifSecond - timedifFirst) < pendingTime);
				
			}
			else
			{
				printf("This function does not be calculated in ");
				printf("given resolution that %ld ns \n",pendingTime);
				return -1;

			}
		
		}

		return resultOfGivenFunction;
	 
}

long double calculatePostfix(long double timeValue)
{
	
	long double resultOfGivenFunction = 0.0;
	int len = strlen(globArgv);
	int tempIsOperator, tempIsNumber, tempIsParam;
	char tempString[100];
	
	while(hasMoreToken() > 0)
		{
			tempIsOperator = isOperator() ;
			tempIsParam    = isParam();
			tempIsNumber   = isNumber();
			len = globNextIter - globPrevIter;
			
			if(tempIsNumber == 1)
			{
				strncpy(tempString,	&globArgv[globPrevIter], len);
				tempString[len] = '\0';
				push(atof(tempString));
			}
			else if(tempIsParam == 1)
			{
				push(timeValue);
			}
			else if(tempIsOperator != 0)
			{
				strncpy(tempString,	&globArgv[globPrevIter], len);
				tempString[len] = '\0';
				if(getPrecedence(tempString) != 3)
				{
					push(evalBin(tempString));
				}
				else if (getPrecedence(tempString) == 3)
				{
					push(evalUna(tempString));
				}
			}
			
			// checking whether over the array size
			if(globNextIter < globLenOfFunc - 1)
				globPrevIter = globNextIter + 1;
			else
			{
				globPrevIter = globNextIter;
				break;
			}
		}
		resultOfGivenFunction = pop();
		reset();
return resultOfGivenFunction;
}

double evalBin(char *op)
{
	int precedenceOfOp = getIndexOfOp(op);
	
	double 	lhs = 0.0, rhs = 0.0, result = 0.0;
	
	rhs = pop();
	lhs = pop();
	
	switch (precedenceOfOp)
	{
		case 2: result = lhs + rhs;
				break;
		case 3: result = lhs - rhs;
				break;
		case 4: result = lhs * rhs;
				break;
		case 5: result = lhs / rhs;
				break;
		case 9: result = pow(lhs,rhs);
				break;
	}
	
	return result;
}

double evalUna(char *op)
{
	int precedenceOfOp = getIndexOfOp(op);
	double result = 0.0,tempValue = 0.0;
	
	tempValue = pop();
	
	switch (precedenceOfOp)
	{
		case 6: result = sin(tempValue);
				break;
		case 7: result = cos(tempValue);
				break;
		case 8: result = exp(tempValue);
				break;
	}
	
	return result;
}

long getNanoSec()
{
	unsigned long timedif;
	struct timespec tpend;
	
	clock_gettime(CLOCK_REALTIME,&tpend);
	timedif = BILLIONM  * (tpend.tv_sec) + tpend.tv_nsec;
	
	return timedif;
}

long double takeIntegration(long startTime)
{
	long double subInterval[STEP+1];
	long double resultOfSubInter = 0.0;
	long double sum1 = 0.0, sum2 = 0.0;
	double h = 0.0;
	int i = 0, j = 0;
	
    h=((double)pendingTime/(double)STEP);
		for(i=0; i<=STEP; ++i)
		{
			subInterval[i] = calculatePostfix((i*h)+startTime);
		}
        for(i=1;i<STEP;i+=2){
            sum1+=subInterval[i];
        }
        sum1*=4;
        for(j=2;j<STEP-1;j+=2){
            sum2+=subInterval[j];
        }
        sum2*=2;
        resultOfSubInter = sum1 + sum2 + 
						   subInterval[STEP]+
						   subInterval[0];
        
   return(pendingTime*(resultOfSubInter/(3*STEP)));
}

long takeDerivation(long startTime)
{
	const double delta = 0.0001; // or similar
    long double x1 = startTime - delta;
    long double x2 = startTime + delta;
    long double y1 = calculatePostfix(x1);
    long double y2 = calculatePostfix(x2);
    
    
    return (y2 - y1) / (x2 - x1);
}

long double mathFunction(char* chrPtrFunction)
{
	long double resultFunc;
	
	resultFunc=postfixCalculator(chrPtrFunction);
	if(resultFunc==-1)
	{
		return -1;
	}
	return resultFunc;	

}
