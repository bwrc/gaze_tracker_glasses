#include "Server.h"
#include <cerrno>
#include <cstring>


namespace gtSocket {


Server::Server() : Communicator() {}


Server::~Server() {

	close(serverFd);

}


bool Server::init(const char *sunPath) {

	if((serverFd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		strErr = std::strerror(errno);
	}

	struct sockaddr_un serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, sunPath);
	int servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if(bind(serverFd, (struct sockaddr *)&serv_addr,servlen) < 0) {
		strErr = std::strerror(errno);
		return false;
	}

	return true;

}


bool Server::start() {

	listen(serverFd, 5);

	struct sockaddr_un cli_addr;
	socklen_t clilen = sizeof(cli_addr);
	communicationFd = accept(serverFd, (struct sockaddr *)&cli_addr, &clilen);

	if(communicationFd < 0) {
		strErr = std::strerror(errno);
		return false;
	}

	return true;

}


}

