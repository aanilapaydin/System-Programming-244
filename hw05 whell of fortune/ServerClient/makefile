CC=gcc

CLIENT=client.c
SERVER=server.c

LIBRARY_SOURCES=-lm -lrt -lpthread

EXECUTABLE_NAME_SERVER=server

EXECUTABLE_NAME_CLIENT=client


RELEASE_PARAMS= -O3 -Wall -Wextra  -pedantic -DNDEBUG

DEBUG_PARAMS= -g3 -o0 -Wall -Wextra -pedantic 


all: release

debug: serverdebug clientdebug
release: serverrelease clientrelease

clean:
	rm -rf *.o
	rm -rf ${EXECUTABLE_NAME_CLIENT} ${EXECUTABLE_NAME_SERVER}

serverdebug:
	${CC} ${DEBUG_PARAMS} ${SERVER} ${LIBRARY_SOURCES} -o ${EXECUTABLE_NAME_SERVER}
	
serverrelease:
	${CC} ${RELEASE_PARAMS} ${SERVER} ${LIBRARY_SOURCES} -o ${EXECUTABLE_NAME_SERVER}

clientdebug:
	${CC} ${DEBUG_PARAMS} ${CLIENT} ${LIBRARY_SOURCES} -o ${EXECUTABLE_NAME_CLIENT}
	
clientrelease:
	${CC} ${RELEASE_PARAMS} ${CLIENT} ${LIBRARY_SOURCES} -o ${EXECUTABLE_NAME_CLIENT}
