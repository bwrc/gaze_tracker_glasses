#include "ResultWriter.h"
#include <sys/stat.h>


ResultWriter::ResultWriter() : DataWriter() {

}


ResultWriter::~ResultWriter() {

	pthread_mutex_lock(&mutex);

		// flush
		std::list<QueueData>::iterator it = queue.begin();
		while(it != queue.end()) {

			write(*it);

			it->release();

			++it;
		}

	queue.clear();

	pthread_mutex_unlock(&mutex);

	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

}


bool ResultWriter::init(const std::string &_parentDir) {

	// store the working dir
	workingDir = _parentDir;

	/**************************************************
	 * Create the result output file
	 **************************************************/
    std::string file = workingDir + "results.res";
    streamResults.open(file.c_str(), std::ofstream::binary);
    if(!streamResults.is_open()) {
        return false;
    }


	// create the mutex
	if(pthread_mutex_init(&mutex, NULL) != 0) {
		return false;
	}

	if(pthread_cond_init(&cond, NULL) != 0) {
		return false;
	}


	return true;

}


bool ResultWriter::addFrames(const CameraFrame *_f1, const CameraFrame *_f2) {

	return true;

}


bool ResultWriter::addResults(const std::vector<char> &buff) {

	if(!isRunning()) {

		return false;

	}

	size_t sz = buff.size();

	char *data = new char[sz];
	memcpy(data, buff.data(), sz);

	pthread_mutex_lock(&mutex); {

        QueueData el(data, sz);
		queue.push_back(el);

		// getElement() might be waiting
		pthread_cond_signal(&cond);

    } pthread_mutex_unlock(&mutex);

	return false;

}


void ResultWriter::run() {

	// loop while alive
	while(isRunning()) {

		// get the oldest element
		QueueData el = getElement();

		// continue if empty
		if(el.empty()) {


			// give add() a chance to execute
			sleepMs(2);

			continue;

		}

		// write to files
		write(el);

		// destroy data
		el.release();

	}

	printf("ResultWriter::run(): bye\n");

}


void ResultWriter::waitForData() {

    // get the system time
    struct timespec ts;

    static const long BILLION = 1e9;
    static const long MILLION = 1e6;

    clock_gettime(CLOCK_REALTIME, &ts);

    int millis = 4;

    // add the requested wait time
    ts.tv_sec  += millis / 1000;
    ts.tv_nsec += (millis % 1000) * MILLION; // millis * 1e6 => nanosecond

    // account for overflow
    if(ts.tv_nsec >= BILLION) {
        ++ts.tv_sec;
        ts.tv_nsec -= BILLION;
    }


    /*****************************************************************
     * Wait on the condition, which must be locked
     *****************************************************************/

    // on success returns zero
    /*int ret =*/ pthread_cond_timedwait(&cond, &mutex, &ts);

}


QueueData ResultWriter::getElement() {

	pthread_mutex_lock(&mutex);

		// wait if empty
        while(queue.size() == 0 && isRunning()) {
            waitForData();
        }

		// if still empty, i.e. the destructor signalled, return
		if(queue.size() == 0) {
			pthread_mutex_unlock(&mutex);
			return QueueData();
		}

		std::list<QueueData>::iterator it = queue.begin();

		const QueueData el = *it;

		// remove the reference from the list
		queue.erase(it);

    pthread_mutex_unlock(&mutex);

	return el;

}


void ResultWriter::write(const QueueData &el) {

    streamResults.write((const char *)el.m_pData, el.m_nSz);

}


int ResultWriter::getBufferState() {

	pthread_mutex_lock(&mutex);
		int ret = (int)queue.size();
	pthread_mutex_unlock(&mutex);

	return ret;

}

