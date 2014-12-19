#include "Client.h"
#include <stdio.h>


int main(int nargs, char *args[]) {

	if(nargs != 2) {

		printf("Give the sun path\n");

		return -1;

	}


	gtSocket::Client client;
	if(!client.init(args[1], 1000)) {

		printf("%s\n", client.getError().c_str());

		return -1;

	}

	printf("Establish connection...");
	fflush(stdout);

	if(!client.start()) {

		printf("%s\n", client.getError().c_str());

		return -1;

	}

	printf("ok\n");

	char userInput[80];
	bzero(userInput, 80);

	while(strcmp(userInput, "quit") != 0) {

		printf("Please enter your message: ");
		bzero(userInput, 80);
		gets(userInput);

		client.send(userInput, strlen(userInput));

	}

	return 0;

}

