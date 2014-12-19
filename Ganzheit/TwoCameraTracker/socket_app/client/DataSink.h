#ifndef DATASINK_H
#define DADASINK_H


#include <list>
#include <vector>
#include <pthread.h>
#include "Client.h"
#include "ResultData.h"
#include "CameraFrameExtended.h"



class DataContainer {

	public:

		enum DATATYPE {
			TYPE_FRAME1,
			TYPE_FRAME2,
			TYPE_TRACK_RESULTS
		};

		DataContainer() {
			data = NULL;
			len = 0;
		}

		char *data;
		int len;

};



class DataSink {

	public:

		DataSink();
		~DataSink();

		bool init(const char *socketName);

		void addResults(ResultData *res);
		void addFrame(CameraFrameExtended *frame, int32_t dataType); // TYPE_FRAME1 or TYPE_FRAME2
		void addFrames(CameraFrameExtended *frames[2]);

		void run();

		void end();

		int getBufferState();

		void write(const DataContainer *dataCont);

	private:

		void add(char *data, int32_t len);

		DataContainer *getDataContainer();

		void destroyDataContainer(DataContainer *dataCont);

		bool alive();

		bool wait(int millis);

		pthread_mutex_t mutex;
		pthread_cond_t cond;
		pthread_t thread;
		pthread_mutex_t mutex_alive;
		volatile bool b_alive;

		std::list<DataContainer *> queueData;

		gtSocket::Client client;

};


#endif

