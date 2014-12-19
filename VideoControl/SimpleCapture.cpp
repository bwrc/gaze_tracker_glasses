#include "SimpleCapture.h"
#include <string.h>


static const int MAX_BUFFER_SZ = 2;


SimpleCapture::SimpleCapture() {

	capdev = NULL;
    bOpened = false;
}


SimpleCapture::~SimpleCapture() {

    if(!bOpened) {
        return;
    }

	delete capdev;


	pthread_mutex_lock(&mutex_frame);

    std::list<CameraFrame *>::iterator it = frameBuffer.begin();
    std::list<CameraFrame *>::iterator itEnd = frameBuffer.end();

    while(it != itEnd) {

        delete *it;
        ++it;

    }

    frameBuffer.clear();

    pthread_cond_signal(&cond_frame);

	pthread_mutex_unlock(&mutex_frame);

	pthread_mutex_destroy(&mutex_frame);
	pthread_cond_destroy(&cond_frame);

}


CameraFrame *SimpleCapture::grabFrame(bool block) {

	pthread_mutex_lock(&mutex_frame);

    if(frameBuffer.size() == 0) {

        int ret_wait = pthread_cond_wait(&cond_frame, &mutex_frame);

        if(ret_wait != 0) {
            pthread_mutex_unlock(&mutex_frame);
            return NULL;
        }

    }

    // at this point there surely is at least one frame in the buffer

    std::list<CameraFrame *>::iterator itOldest = frameBuffer.begin();
    CameraFrame *ret = *itOldest;

    frameBuffer.erase(itOldest);

	pthread_mutex_unlock(&mutex_frame);

	return ret;

}


// bool SimpleCapture::grabFrame(CameraFrame &f, bool block) {

// 	pthread_mutex_lock(&mutex_frame);

//     int ret_wait = pthread_cond_wait(&cond_frame, &mutex_frame);

//     if(ret_wait != 0) {
//         return false;
//     }

//     memcpy(&f, frame, sizeof(CameraFrame));


//     frame = NULL;

// 	pthread_mutex_unlock(&mutex_frame);

// 	return true;

// }


bool SimpleCapture::open(const std::string &path,
						 const int width,
						 const int height,
						 const int framerate,
						 const Format format) {

	/************************************************************************
	 * Create the mutex and the condition variable
	 ************************************************************************/

	int ret = pthread_mutex_init(&mutex_frame, NULL);
	if(ret != 0) {
		printf("SimpleCapture::open(): Could not create the mutex");
		return false;
	}

	ret = pthread_cond_init(&cond_frame, NULL);
	if(ret != 0) {
		printf("SimpleCapture::open(): Could not create the condition variable");
		return false;
	}

    bOpened = true;


	/************************************************************************
	 * Create video stream
	 ************************************************************************/
	capdev = new CaptureDevice();


	bool bRet = capdev->init(path,
							 width,
							 height,
							 framerate,
							 format,
							 this,  // this is the FrameReceiver
                             0);

	return bRet;

}


void SimpleCapture::frameReceived(const CameraFrame *_frame, int id) {

    // copy the new frame
    //		frame = new CameraFrame(*_frame);
    CameraFrame *frame = new CameraFrame(_frame->w,
                                         _frame->h,
                                         _frame->bpp,
                                         _frame->data,
                                         _frame->sz,
                                         _frame->format,
                                         true,			 // copy data
                                         true);		     // become parent, i.e. destroy data in destructor

	pthread_mutex_lock(&mutex_frame);

    // if the buffer is full, delete the oldest
    if((int)frameBuffer.size() == MAX_BUFFER_SZ) {

        std::list<CameraFrame *>::iterator itOldest = frameBuffer.begin();
        delete *itOldest;
        frameBuffer.erase(itOldest);

    }

    frameBuffer.push_back(frame);

    // signal because grabFrame() might be waiting
    pthread_cond_signal(&cond_frame);

	pthread_mutex_unlock(&mutex_frame);

}

