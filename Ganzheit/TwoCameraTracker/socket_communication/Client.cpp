#include "Client.h"
#include <cerrno>
#include <cstring>


namespace gtSocket {


Client::Client() : Communicator() {

}


bool Client::init(const char *sunPath) {

	bzero((char *)&serv_addr,sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;

	strcpy(serv_addr.sun_path, sunPath);

	/*
	 * http://publib.boulder.ibm.com/infocenter/pseries/v5r3/index.jsp?topic=/com.ibm.aix.progcomm/doc/progcomc/skt_types.htm
	 * In the UNIX domain, the SOCK_STREAM socket type works like a pipe.
	 * In the Internet domain, the SOCK_STREAM socket type is implemented
	 * on the Transmission Control Protocol/Internet Protocol (TCP/IP)
	 * protocol.
	 *
	 * Which means that, no overhead will be introduced, when AF_UNIX
	 * is being used.
	 */
	if((communicationFd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {

		strErr = std::strerror(errno);
		return false;

	}

	return true;

}


bool Client::start() {

	int servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if(connect(communicationFd, (struct sockaddr *)&serv_addr, servlen) < 0) {
		strErr = std::strerror(errno);
		return false;
	}

	return true;

}


}

