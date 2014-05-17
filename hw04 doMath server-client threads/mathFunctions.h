#define LEN_OF_OP 10
#define OPROW 10
#define OPCOL 10
#define STEP 10
#define THOUSAND 1000
#define BILLIONM  1000000000
#define FIFOARG 1


char *globArgv;
int globLenOfFunc = 0;
int globPrevIter  = 0;
int globNextIter  = 0;
long currentParam;
long pendingTime;

char *OPERATORS[] = {"(",")","+","-","*","/","sin","cos","exp","^"};
int  PRECEDENCE[] = {-1,-1,1,1,2,2,3,3,3,4};

char postfix[100][100];
char opStack[OPROW][OPCOL];
char topOfStack[OPROW];
long double resultOfPostfix[OPROW];
int sizePostFix = 0;
int sizeOpStack = 0;
int sizeOfResultStack = 0;
int closeParanFlag = 0;
int paranIndex = 0;


long double mathFunction(char* chrPtrFunction);
int isDigit(char digit);
int isNumber();
int isOperator();
int isParam();
int hasMoreToken();
void pushOp();
void push(double newValue);
void popOp();
double pop();
void stringBuilder(char *newString);
void peekOp();
double peek();
int getPrecedence( char *newOperator);
int getIndexOfOp( char *newOperator);
void processOperator();
int convertToPostfix();
long double postfixCalculator(char* chrPtrFunction);
void reset();
double evalBin(char *op);
double evalUna(char *op);
long getNanoSec();
long double calculatePostfix(long double timeValue);
long double takeIntegration(long startTime);
long takeDerivation(long startTime);
