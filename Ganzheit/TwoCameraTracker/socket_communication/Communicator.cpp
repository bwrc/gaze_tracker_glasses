#include "Communicator.h"


namespace gtSocket {


Communicator::Communicator() {

}


Communicator::~Communicator() {

	close(communicationFd);

}


int Communicator::receive(char *buffer, int nRequested, bool block) {

	// bytes read so far
	int nRead = 0;

	// blockable or not?
	if(block) {

		// bytes still to read
		int nLeftToRead = nRequested;

		while(nRead != nRequested) {

			int nReadThisTime = read(communicationFd, buffer + nRead, nLeftToRead);

			if(nReadThisTime < 0) {

				return -1;

			}

			nRead += nReadThisTime;
			nLeftToRead = nRequested - nRead;

		}

	}
	else {

		nRead = read(communicationFd, buffer, nRequested);

	}

	return nRead;

}


int Communicator::send(const char *buffWrite, int nToWrite) {

	int nWritten = write(communicationFd, buffWrite, nToWrite);

	return nWritten;

}


}

