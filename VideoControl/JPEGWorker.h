#ifndef JPEGWORKER_H
#define JPEGWORKER_H


#include "StreamWorker.h"



class JPEGWorker : public StreamWorker {

	public:

		JPEGWorker();
		virtual ~JPEGWorker();

	protected:

		CameraFrame *process(CameraFrame *img_compr);

};


#endif

