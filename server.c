#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "relay.h"
#include "server.h"

/*
 * Double underscore is used to underline that variable is static
 */

#define MODULE1 0
#define MODULE2 1

#define MODULE_ON 1
#define MODULE_OFF 0

#define MAX_BUFFER_SIZE 1024

static pthread_mutex_t *mutex;


/*
 * Struct to handle actions of program
 */
struct __action {
	int moduleNumber;
	int method;
};

enum {
	GET_MODULE = 1,
	SET_MODULE = 2,
};

static int executeAction(struct __action *action)
{
	void (*switchState)(void);
	int (*getState)(void);

	switch(action->moduleNumber) {
		case MODULE1:
			switchState = switch1Relay;
			getState = getRelay1State;
			break;
		case MODULE2:
			switchState = switch2Relay;
			getState = getRelay2State;
			break;
	}

	switch(action->method) {
		case SET_MODULE:
			switchState();
			return 0;
		case GET_MODULE:
			return getState();
		defualt:
			printf("UNKNOWN ACTION\n");
			return -1;
	}
}

static int parseReceivedData(const char *buffer, struct __action *action)
{
	int ret = 0;
	int result;
	int module;
	char *newLine;

	result = strncmp(buffer, "GET", 3);
	
	if (result == 0) {
		action->method = GET_MODULE;	
		goto continue_parsing;
	}

	result = strncmp(buffer, "SET", 3);

	if (result == 0) {
		action->method = SET_MODULE;	
		goto continue_parsing;
	}

	if (result != 0) {
		action->method = 0;
		action->moduleNumber = 0;
		return -1;
	}

continue_parsing:
	
	/* 
	 * At new line is number of module
	 */
	newLine = strstr(buffer, "\n");
	module =  atoi(&newLine[1]);

	switch (--module) {
		case 0:
			action->moduleNumber = MODULE1;
			break;
		case 1:
			action->moduleNumber = MODULE2;
			break;
		default:
			printf("WRONG MODULE NUMBER\n");
			ret = -1;
			break;
	}

	return ret;
}

/*
 * This functions will be executex in separate thread
 */
void* handleClient(void *arg)
{
	/*
	 * Make sure that value won't be changed
	 */
	pthread_mutex_lock(mutex);
	int socketfd = *(int *) arg;
	pthread_mutex_unlock(mutex);

	int error;
	int bytesReceived;
	char recvBuffer[MAX_BUFFER_SIZE];
	struct __action action;
	
	bytesReceived = recv(socketfd, recvBuffer, MAX_BUFFER_SIZE, 0);
	if (bytesReceived == -1) {
		printf("RECEIVER ERROR\n");
		// TODO handle this properly
		// for now fall through
	}

	error = parseReceivedData(recvBuffer, &action);
	if (error) {
		printf("PARSING GONE WRONG\n");
	}
	
	/*
	 * Critical section
	 */
	pthread_mutex_lock(mutex);
	executeAction(&action);
	pthread_mutex_unlock(mutex);
}

static int __socketfd;
static volatile int __running;

int createSocket(int port)
{
	struct sockaddr_in addres;
	socklen_t sockSize;
	int error;

	error = relayModuleInit(15, 0);

	__socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (__socketfd == -1) {
		printf("Couldnt create socket\n");
		return -1;
	}

	/*
	 * Specify addres of socket
	 */
	addres.sin_family = AF_INET;	
	addres.sin_port = htons(port);	
	addres.sin_addr.s_addr = INADDR_ANY;

	/*
	 * Calculate size of addres
	 */
	sockSize = sizeof(addres);

	error = bind(__socketfd, (struct sockaddr *) &addres, sockSize);
	if (error) {
		printf("Couldnt bind socket to port\n");
		return -1;
	}

	return 0;
}

void startListnening(void)
{
	int error;
	int bytesReceived;
	int bytesToSend;
	int parseResult;
	struct sockaddr_in addres;
	socklen_t sockSize;
	pthread_t thread;

	sockSize = sizeof(addres);

	__running = 1;

	error = listen(__socketfd, 5);	
	if (error) {
		__running = 0;
		printf("Socket couldnt start listening\n");
		return;
	}

	while(__running) {
		int clientSocket = accept(__socketfd, (struct sockaddr *) &addres, &sockSize);

		if (clientSocket == -1) {
			if (__running) printf("Couldnt accept connection\n");
			continue;
		}

		/*
		 * TODO: Implement handleClient function
		 */
		error = pthread_create(&thread, NULL, handleClient, (void *) &clientSocket);
		if (error) {
			printf("Couldnt create thread to handle user requests\n");
			continue;
		}
	}
}

void closeSocket(void)
{
	close(__socketfd);
}
