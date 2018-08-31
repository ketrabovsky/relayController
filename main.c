#include <stdio.h>

#include "server.h"

#define PORT 5050

int main()
{
	int error;
	
	error = createSocket(PORT);
	if (error) {
		printf("Socket creation error\n");
		return -1;
	}

	/*
	 * Start accepting incoming connections and handle them
	 */
	startListnening();

	/*
	 * On exit close socket
	 */
	closeSocket();

	return 0;
}
