/* a server in the unix domain.  The pathname of 
   the socket address is passed as an argument */
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/un.h>
#include <stdio.h>



void error(const char *);


int main(int nargs, char *argv[]) {

	if(nargs != 2) {

		printf("Give the sun path number\n");

		return -1;

	}


	int sockfd;
	if((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		error("creating socket");
	}

	struct sockaddr_un serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, argv[1]);
	int servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if(bind(sockfd,(struct sockaddr *)&serv_addr,servlen) < 0) {
		error("binding socket");
	}

	listen(sockfd, 5);

	struct sockaddr_un cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	int newsockfd = accept(sockfd,(struct sockaddr *)&cli_addr, &clilen);

	if(newsockfd < 0) {
		error("accepting");
	}

	char buf[80];
	int n = read(newsockfd, buf, 80);

	printf("A connection has been established\n");
	write(1, buf, n); // 1 ==> stdout
	write(newsockfd, "I got your message\n", 19);

	close(newsockfd);
	close(sockfd);

	return 0;

}


void error(const char *msg) {

	perror(msg);
	exit(0);

}

