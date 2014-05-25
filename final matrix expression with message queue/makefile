CC=gcc

INVERSE=inverseTaking.c
LU=luDecomposition.c
GEN=matrixGenerator.c

LIBRARY_SOURCES=-lm -lrt -lpthread

EXECUTABLE_NAME_INVERSE=inverseTaking

EXECUTABLE_NAME_LU=luDecomposition

EXECUTABLE_NAME_GEN=matrixGenerator

RELEASE_PARAMS= -O3 -Wall -Wextra  -pedantic -DNDEBUG

DEBUG_PARAMS= -g3 -o0 -Wall -Wextra -pedantic 


all: release

debug: ludebug gendebug invdebug
release: lurelease genrelease invrelease

clean:
	rm -rf *.o
	rm -rf ${EXECUTABLE_NAME_INVERSE} ${EXECUTABLE_NAME_LU} ${EXECUTABLE_NAME_GEN}

ludebug:
	${CC} ${DEBUG_PARAMS} ${LU} ${LIBRARY_SOURCES} -o ${EXECUTABLE_NAME_LU}
	
lurelease:
	${CC} ${RELEASE_PARAMS} ${LU} ${LIBRARY_SOURCES} -o ${EXECUTABLE_NAME_LU}

gendebug:
	${CC} ${DEBUG_PARAMS} ${GEN} ${LIBRARY_SOURCES} -o ${EXECUTABLE_NAME_GEN}
	
genrelease:
	${CC} ${RELEASE_PARAMS} ${GEN} ${LIBRARY_SOURCES} -o ${EXECUTABLE_NAME_GEN}

invdebug:
	${CC} ${DEBUG_PARAMS} ${INVERSE} ${LIBRARY_SOURCES} -o ${EXECUTABLE_NAME_INVERSE}
	
invrelease:
	${CC} ${RELEASE_PARAMS} ${INVERSE} ${LIBRARY_SOURCES} -o ${EXECUTABLE_NAME_INVERSE}
