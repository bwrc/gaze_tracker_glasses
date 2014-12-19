#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H


#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/un.h>
#include <string>



namespace gtSocket {


class Communicator {

	public:

		Communicator();

		virtual ~Communicator();

		int receive(char *buffer, int nToRead, bool block);
		int send(const char *buffWrite, int nToWrite);

	protected:

		/* Socket file descriptor */
		int communicationFd;

};


}

#endif

