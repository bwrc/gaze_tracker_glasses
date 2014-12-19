#include "Server.h"
#include <stdio.h>


int main(int nargs, char *args[]) {

	if(nargs != 2) {

		printf("Give the sun path\n");

		return -1;

	}


	gtSocket::Server server;
	if(!server.init(args[1], 1000)) {

		printf("%s\n", server.getError().c_str());

		return -1;

	}

	printf("Establish connection...");
	fflush(stdout);

	if(!server.start()) {

		printf("%s\n", server.getError().c_str());

		return -1;

	}

	printf("ok\n");

	const char *buff = NULL;

	do {

		int nRead = server.receive(80, false);
		buff = server.getBuffer();
		write(1, buff, nRead); // 1 ==> stdout
		printf("\n");

	}
	while(memcmp(buff, "quit", 4) != 0);

	return 0;

}

