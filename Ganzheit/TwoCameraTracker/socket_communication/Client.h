#ifndef CLIENT_H
#define CLIENT_H

// a client in the unix domain 
//#include <sys/types.h>
//#include <unistd.h>
//#include <stdlib.h>
//#include <sys/socket.h>
//#include <sys/un.h>
//#include <string>

#include "Communicator.h"


namespace gtSocket {


class Client : public Communicator {

	public:

		Client();

		bool init(const char *sunPath);
		bool start();

		const std::string &getError() {return strErr;}

	private:

		std::string strErr;

		struct sockaddr_un serv_addr;

};


}


#endif

