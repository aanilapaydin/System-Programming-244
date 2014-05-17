CC=gcc

SERVER=localServer.c

LIBRARY_SOURCES=-lm -lrt -lpthread

EXECUTABLE_NAME_SERVER=localServer

RELEASE_PARAMS= -O3 -Wall -Wextra  -pedantic -DNDEBUG

DEBUG_PARAMS= -g3 -o0 -Wall -Wextra -pedantic 


all: release

debug: serverdebug
release: serverrelease

clean:
	rm -rf *.o
	rm -rf ${EXECUTABLE_NAME_CLIENT} ${EXECUTABLE_NAME_SERVER}

serverdebug:
	${CC} ${DEBUG_PARAMS} ${SERVER} ${LIBRARY_SOURCES} -o ${EXECUTABLE_NAME_SERVER}
	
serverrelease:
	${CC} ${RELEASE_PARAMS} ${SERVER} ${LIBRARY_SOURCES} -o ${EXECUTABLE_NAME_SERVER}

