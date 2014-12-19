#include "VideoWriter.h"
#include <sys/stat.h>


static const long BILLION = 1e9;

/* Duration between backups in seconds */
static const long DUR_BACKUP = 2 * 60;


VideoWriter::VideoWriter() : DataWriter() {

	countBackup = 0;

}


VideoWriter::~VideoWriter() {

	pthread_mutex_lock(&mutex);

		// flush
		std::list<QueueElement>::iterator it = queue.begin();
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


bool VideoWriter::init(const std::string &_parentDir) {

	/*
	 * The output directory must not exists. If this is true
	 * get the current time and create a new directory
	 * inside it whose name is based on the current date and time.
	 */

	// get the current date and time
	time_t t = time(0);   // get time now
	const struct tm *now = localtime(&t);
	int year	= now->tm_year + 1900;
	int month	= now->tm_mon + 1;
	int day		= now->tm_mday;
	int hour	= now->tm_hour;
	int min		= now->tm_min;
	int sec		= now->tm_sec;

	/**********************************************************
	 * Create the name for the output direcoty. Example:
	 * parent_dir/20120601T163023/
	 **********************************************************/
	char strOputDir[PATH_MAX];
	sprintf(strOputDir, "%s/%d%.2d%.2dT%.2d%.2d%.2d/", _parentDir.c_str(), year, month, day, hour, min, sec);


	// store the working dir
	workingDir = std::string(strOputDir);


    /*
     * Create the working dir
     */
    if(!createFolder(workingDir)) {
        return false;
    }

	/**************************************************
	 * Create the output files
	 **************************************************/
	if(!createNewFiles()) {
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


bool VideoWriter::createNewFiles() {

	printf("VideoWriter::createNewFiles(): creating new files\n");

	/****************************************
	 * First create the new folder
	 ****************************************/

	std::stringstream ss;
	ss << workingDir << "part" << countBackup++ << "/";

	std::string folderPath = ss.str();

	if(!createFolder(folderPath)) {

		return false;

	}


	/****************************************
	 * Now create the files
	 ****************************************/

    // create new video files if required

    std::string fileCamEye = std::string(folderPath);
    fileCamEye.append("camera1.mjpg");

    std::string fileCamScene = std::string(folderPath);
    fileCamScene.append("camera2.mjpg");

    // eye cam file
    streamEyeCam.close();
    streamEyeCam.open(fileCamEye.c_str(), std::ofstream::binary);

    // check if the file is open
    if(!streamEyeCam.is_open()) {
        printf("VideoWriter::init(): Could not create %s\n", fileCamEye.c_str());
        return false;
    }

    // scene cam file
    streamSceneCam.close();
    streamSceneCam.open(fileCamScene.c_str(), std::ofstream::binary);

    // check if the file is open
    if(!streamSceneCam.is_open()) {
        printf("VideoWriter::init(): Could not create %s\n", fileCamScene.c_str());
        return false;
    }


	std::string fileResults = std::string(folderPath);
	fileResults.append("results.res");


	// result file
	streamResults.close();
	streamResults.open(fileResults.c_str(), std::ofstream::binary);

	// check if the file is open
	if(!streamResults.is_open()) {
		printf("VideoWriter::init(): Could not create %s\n", fileResults.c_str());
		return false;
	}


	return true;

}


bool VideoWriter::addFrames(const CameraFrame *_f1, const CameraFrame *_f2) {

    // if this thread is not running, do not add
	if(!isRunning()) {
		return false;
	}


	// create element 1
	int sz1 = _f1->sz;
	char *data1 = new char[sz1];
	memcpy(data1, _f1->data, sz1);
	QueueElement el1(data1, sz1, QueueElement::TYPE_FRAME1);

	// create element 2
	int sz2 = _f2->sz;
	char *data2 = new char[sz2];
	memcpy(data2, _f2->data, sz2);
	QueueElement el2(data2, sz2, QueueElement::TYPE_FRAME2);


	pthread_mutex_lock(&mutex);

		// place the elements to the queue
		queue.push_back(el1);
		queue.push_back(el2);

		// getElement() might be waiting
		pthread_cond_signal(&cond);

	pthread_mutex_unlock(&mutex);

	return true;

}


bool VideoWriter::addResults(const std::vector<char> &buff) {

	if(!isRunning()) {

		return false;

	}

	size_t sz = buff.size();

	char *data = new char[sz];
	memcpy(data, buff.data(), sz);

	pthread_mutex_lock(&mutex);

		QueueElement el(data, sz, QueueElement::TYPE_RESULTS);
		queue.push_back(el);

		// getElement() might be waiting
		pthread_cond_signal(&cond);

	pthread_mutex_unlock(&mutex);

	return false;

}


void VideoWriter::run() {

	gettimeofday(&timeStart, NULL);

	// loop while alive
	while(isRunning()) {

		/* Every predefined interval, backups will be made */
		if(DUR_BACKUP == elapsedSeconds()) {

			// indicates that the backups are being done
			zeroTimer();

			// create new output files for the streams
			if(!createNewFiles()) {

				// this thread must exit
				killSelf();

				break;

			}

		}


		// get the oldest element
		QueueElement el = getElement();

		// continue if empty
		if(el.empty()) {

			// give add() a chance to execute
			sleepMs(2);

			continue;

		}

		// write to files
		write(el);

		// destroy the frame pair
		el.release();

	}

	printf("VideoWriter::run(): bye\n");

}


QueueElement VideoWriter::getElement() {

	pthread_mutex_lock(&mutex);

		// wait if empty
        while(queue.size() == 0 && isRunning()) {
            waitForData();
        }

		// if still empty, i.e. the destructor signalled, return
		if(queue.size() == 0) {
			pthread_mutex_unlock(&mutex);
			return QueueElement();
		}

		std::list<QueueElement>::iterator it = queue.begin();

		const QueueElement el = *it;

		// remove the reference from the list
		queue.erase(it);

	pthread_mutex_unlock(&mutex);

	return el;

}


void VideoWriter::write(const QueueElement &el) {

	switch(el.type) {

		case QueueElement::TYPE_FRAME1: {

			streamEyeCam.write((const char *)el.data, el.size);

			break;

		}

		case QueueElement::TYPE_FRAME2: {

			streamSceneCam.write((const char *)el.data, el.size);

			break;

		}

		case QueueElement::TYPE_RESULTS: {

			streamResults.write((const char *)el.data, el.size);

			break;

		}

		default: break; // should never be reached

	}

}


long VideoWriter::elapsedSeconds() {

	struct timeval t;
	gettimeofday(&t, NULL);

	return t.tv_sec - timeStart.tv_sec;

}


void VideoWriter::zeroTimer() {

	gettimeofday(&timeStart, NULL);

}


bool VideoWriter::createFolder(const std::string &oputDir) {

	if(oputDir.size() >= PATH_MAX) {
		printf("VideoWriter::createFolder(): output directory name too long\n%s\n", oputDir.c_str());
		return false;
	}


	/*****************************************
	 * Now create the folder
	 *****************************************/
	struct stat myStat;
	if((stat(oputDir.c_str(), &myStat) != 0) || (((myStat.st_mode) & S_IFMT) != S_IFDIR)) {

		int ret = mkdir(oputDir.c_str(), 0777);

		// see if the creation was successfull
		if(ret != 0) {

			printf("Could not create %s\n", oputDir.c_str());
			return false;

		}

	}

	/*
	 * The creation was unsuccessfull
	 */
	else {

		printf("Could not create the output directory '%s'\n", oputDir.c_str());
		return false;
	}


	return true;

}


int VideoWriter::getBufferState() {

	pthread_mutex_lock(&mutex);
		int ret = (int)queue.size();
	pthread_mutex_unlock(&mutex);

	return ret;

}


void VideoWriter::waitForData() {

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


