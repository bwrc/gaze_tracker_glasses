#ifndef SERVER_H
#define SERVER_H


#include "Communicator.h"


namespace gtSocket {


class Server : public Communicator {

	public:

		Server();
		~Server();

		bool init(const char *sunPath);

		bool start();

		const std::string &getError() {return strErr;}

	private:

		/* Error message */
		std::string strErr;

		/* Server socket file descriptor */
		int serverFd;

};



}


#endif

