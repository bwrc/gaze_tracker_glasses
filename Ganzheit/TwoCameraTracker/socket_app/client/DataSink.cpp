#include "DataSink.h"
#include "BinaryResultParser.h"
#include <stdio.h>


static const long BILLION = 1e9;


static void *thred_fnct(void *arg) {

	DataSink *_this = (DataSink *)arg;
	_this->run();

	pthread_exit(NULL);

}


DataSink::DataSink() {

	b_alive = false;

}


DataSink::~DataSink() {

	// flush
	std::list<DataContainer *>::iterator it = queueData.begin();
	while(it != queueData.end()) {

		DataContainer *data = *it;

		write(data);

		destroyDataContainer(data);

		++it;
	}

	pthread_mutex_lock(&mutex);

		// signal finally
		pthread_cond_signal(&cond);

	pthread_mutex_unlock(&mutex);


	pthread_mutex_destroy(&mutex_alive);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

}


bool DataSink::init(const char *socketFile) {

	printf("DataSink::init(): Establishing connection...");
	fflush(stdout);
	if(!client.init(socketFile)) {
		printf("failed\n");
		return false;
	}

	if(!client.start()) {
		printf("failed\n");
		return false;
	}

	printf("ok\n");


	// create the mutexe
	if(pthread_mutex_init(&mutex, NULL) != 0) {
		return false;
	}

	if(pthread_cond_init(&cond, NULL) != 0) {
		return false;
	}


	if(pthread_mutex_init(&mutex_alive, NULL) != 0) {
		return false;
	}

	// set to true, because we create the the thread next
	b_alive = true;


	// thread attributes
	pthread_attr_t attr;

	/*
	 * initialize and set thread detached attribute.
	 * On some systems the thread will not be created
	 * as joinable, so do it explicitly. It is more
	 * portable this way
	 */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	// create the thread
	int success = pthread_create(&thread, &attr, &thred_fnct, (void *)this);

	// destroy the attributes
	pthread_attr_destroy(&attr);

	// was the thread creation successfull...
	if(success != 0) {

		// nope...

		// we are not running
		b_alive = false;

		return false;
	}

	// yes it was ...

	return true;

}


/* The mutex must be locked before a call to this function */
void DataSink::add(char *data, int32_t len) {

	queueData.push_back(new DataContainer());

	DataContainer *dataCont = queueData.back();
	dataCont->data = data;
	dataCont->len = len;

}


void DataSink::addResults(ResultData *res) {

	pthread_mutex_lock(&mutex);

		std::vector<char> buff;
		BinaryResultParser::resDataToBuffer(*res, buff);

		int len = 4 + buff.size();

		char *newData = new char[len];

		int32_t dataType = DataContainer::TYPE_TRACK_RESULTS;

		newData[0] = (dataType & 0x000000FF);
		newData[1] = (dataType & 0x0000FF00) >> 8;
		newData[2] = (dataType & 0x00FF0000) >> 16;
		newData[3] = (dataType & 0xFF000000) >> 24;

		memcpy(newData + 4, buff.data(), buff.size());

		add(newData, len);

		// write() might be waiting
		pthread_cond_signal(&cond);

	pthread_mutex_unlock(&mutex);

}


// TYPE_FRAME1 or TYPE_FRAME2
void DataSink::addFrame(CameraFrameExtended *frame, int32_t dataType) {

	pthread_mutex_lock(&mutex);

		// type + size + format + id + data
		int len = 4 + 4 + 4 + 4 + frame->sz;

		char *newData = new char[len];


		// type
		newData[0] = (dataType & 0x000000FF);
		newData[1] = (dataType & 0x0000FF00) >> 8;
		newData[2] = (dataType & 0x00FF0000) >> 16;
		newData[3] = (dataType & 0xFF000000) >> 24;


		// size, size of the data, including this 4-byte size info + 4-byte format + 4-byte id
		int32_t size = frame->sz + 4 + 4 + 4;

		newData[4] = (size & 0x000000FF);
		newData[5] = (size & 0x0000FF00) >> 8;
		newData[6] = (size & 0x00FF0000) >> 16;
		newData[7] = (size & 0xFF000000) >> 24;


		// format
		int32_t format = frame->format;

		newData[8]  = (format & 0x000000FF);
		newData[9]  = (format & 0x0000FF00) >> 8;
		newData[10] = (format & 0x00FF0000) >> 16;
		newData[11] = (format & 0xFF000000) >> 24;


		// id
		int32_t id = frame->id;

		newData[12]  = (id & 0x000000FF);
		newData[13]  = (id & 0x0000FF00) >> 8;
		newData[14] = (id & 0x00FF0000) >> 16;
		newData[15] = (id & 0xFF000000) >> 24;

		memcpy(newData + 16, frame->data, frame->sz);

		add(newData, len);


		// write() might be waiting
		pthread_cond_signal(&cond);

	pthread_mutex_unlock(&mutex);

}


// Order must be TYPE_FRAME1 then TYPE_FRAME2
void DataSink::addFrames(CameraFrameExtended *frames[2]) {

	pthread_mutex_lock(&mutex);

		// write() might be waiting
		pthread_cond_signal(&cond);


		// type + size + format + id + data
		int len = 2 * (4 + 4 + 4 + 4) + frames[0]->sz + frames[1]->sz;

		char *newData = new char[len];
		char *ptr = newData;
		int32_t dataTypes[2] = {DataContainer::TYPE_FRAME1, DataContainer::TYPE_FRAME2};

		for(int i = 0; i < 2; ++i) {

			const CameraFrameExtended *curFrame = frames[i];

			int32_t type = dataTypes[i];

			// type
			ptr[0] = (type & 0x000000FF);
			ptr[1] = (type & 0x0000FF00) >> 8;
			ptr[2] = (type & 0x00FF0000) >> 16;
			ptr[3] = (type & 0xFF000000) >> 24;


			// size, size of the data, including this 4-byte size info + 4-byte format + 4-byte id
			int32_t size = curFrame->sz + 4 + 4 + 4;

			ptr[4] = (size & 0x000000FF);
			ptr[5] = (size & 0x0000FF00) >> 8;
			ptr[6] = (size & 0x00FF0000) >> 16;
			ptr[7] = (size & 0xFF000000) >> 24;


			// format
			int32_t format = curFrame->format;

			ptr[8]  = (format & 0x000000FF);
			ptr[9]  = (format & 0x0000FF00) >> 8;
			ptr[10] = (format & 0x00FF0000) >> 16;
			ptr[11] = (format & 0xFF000000) >> 24;


			// id
			int32_t id = curFrame->id;

			ptr[12]  = (id & 0x000000FF);
			ptr[13]  = (id & 0x0000FF00) >> 8;
			ptr[14] = (id & 0x00FF0000) >> 16;
			ptr[15] = (id & 0xFF000000) >> 24;

			memcpy(ptr + 16, curFrame->data, curFrame->sz);

			ptr += 4 + 4 + 4 + 4 + curFrame->sz;

		}

		add(newData, len);

	pthread_mutex_unlock(&mutex);

}


void DataSink::run() {

	// loop while alive
	while(alive()) {

		// get the oldest frame pair
		DataContainer *data = getDataContainer();

		// continue if NULL
		if(data == NULL) {
			continue;
		}

		// write to the client socket
		write(data);

		// destroy the data container
		destroyDataContainer(data);

		// give add() a chance to execute
		wait(2);

	}

	printf("DataSink::run(): bye\n");

}


DataContainer *DataSink::getDataContainer() {

	pthread_mutex_lock(&mutex);

		// wait if empty
		if(queueData.size() == 0) {
			pthread_cond_wait(&cond, &mutex);
		}

		// if still empty, i.e. the destructor signalled, return
		if(queueData.size() == 0) {
			pthread_mutex_unlock(&mutex);
			return NULL;
		}

		std::list<DataContainer *>::iterator it = queueData.begin();

		DataContainer *data = *it;

		// remove the reference from the list
		queueData.erase(it);

	pthread_mutex_unlock(&mutex);

	return data;

}


void DataSink::write(const DataContainer *dataCont) {

	// write to the client socket
	client.send(dataCont->data, dataCont->len);

}


void DataSink::destroyDataContainer(DataContainer *dataCont) {

	delete[] dataCont->data;

	delete dataCont;

}


bool DataSink::wait(int millis) {

	// get the system time
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	// add the requested wait time
	ts.tv_sec	+= millis/1000;
	ts.tv_nsec	+= (millis % 1000) * 1000000;

	// account for overflow
	if(ts.tv_nsec >= BILLION) {
		ts.tv_sec++;
		ts.tv_nsec -= BILLION;
	}

	/*****************************************************************
	 * Wait on the condition
	 *****************************************************************/

	pthread_mutex_lock(&mutex);

	// on success returns zero
	int ret = pthread_cond_timedwait(&cond, &mutex, &ts);

	pthread_mutex_unlock(&mutex);

	return (bool)(ret == 0);	// ret == 0 means we were signalled
}


bool DataSink::alive() {

	pthread_mutex_lock(&mutex_alive);
		bool ret = b_alive;
	pthread_mutex_unlock(&mutex_alive);

	return ret;
}


void DataSink::end() {

	pthread_mutex_lock(&mutex_alive);

		b_alive = false;

		pthread_mutex_lock(&mutex);

		// signal finally
		pthread_cond_signal(&cond);

		pthread_mutex_unlock(&mutex);

	pthread_mutex_unlock(&mutex_alive);


	// wait for the thread to finish
	pthread_join(thread, NULL);

}


int DataSink::getBufferState() {

	pthread_mutex_lock(&mutex);
		int ret = (int)queueData.size();
	pthread_mutex_unlock(&mutex);

	return ret;

}

