#include "StreamWorker.h"



void *stream_worker(void *arg) {

	StreamWorker *_this = (StreamWorker *)arg;
	_this->run();

	pthread_exit(NULL);

}



StreamWorker::StreamWorker() {

	// not runnign  initially
	b_running = false;

	// create the protectors
	pthread_mutex_init(&mutex_list, NULL);
	pthread_cond_init(&cond_list, NULL);

	pthread_mutex_init(&mutex_running, NULL);

}


StreamWorker::~StreamWorker() {

	// delete all memory pointed by the list elements
	pthread_mutex_lock(&mutex_list);

		std::list<CameraFrame *>::iterator it = frames.begin();

		while(it != frames.end()) {

			delete *it;

			++it;

		}

	pthread_mutex_unlock(&mutex_list);


	// destroy the protectors
	pthread_mutex_destroy(&mutex_list);
	pthread_cond_destroy(&cond_list);

	pthread_mutex_destroy(&mutex_running);

}


bool StreamWorker::init(WorkerCBHandler *_cb_handler, int _max_n_frames, void *_user_data) {

	// copy the user data
	user_data = _user_data;

	// copy the handler pointer
	cb_handler = _cb_handler;

	// get the maximum number of frames
	max_n_frames = _max_n_frames;

	if(max_n_frames <= 0 || cb_handler == NULL) {
		return false;
	}

	return true;

}


bool StreamWorker::start() {

	// we running
	b_running = true;

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
	int success = pthread_create(&thread, &attr, &stream_worker, (void *)this);

	// destroy the attributes
	pthread_attr_destroy(&attr);

	// was the thread creation successfull...
	if(success != 0) {

		// nope...

		// we are not running
		b_running = false;

		return false;
	}

	// yes it was ...

	return true;

}


void StreamWorker::end() {

	// set running state to false, we are no longer running
	{
		MutexLocker mlrunning(&mutex_running);
		b_running = false;
	}


	/*
	 * signal the list protector that we are not running anymore.
	 * It might have gone to a waiting state if the getFrame()
	 * sees that the list is empty.
	 */
	{
		MutexLocker mllist(&mutex_list);
		pthread_cond_signal(&cond_list);
	}

	// wait for the thread to finish
	pthread_join(thread, NULL);

}


void StreamWorker::add(CameraFrame *_frame) {

	// protect the list
	MutexLocker mllist(&mutex_list);

	// the buffer is full, delete and drop the incoming frame
	if((int)frames.size() == max_n_frames) {

		delete _frame;

	}
	else {

		// add the frame to the list
		frames.push_back(_frame);

	}

	// signal that we have a frame, getNextFrame() might be waiting
	pthread_cond_signal(&cond_list);

}


CameraFrame *StreamWorker::getNextFrame() {

	// protect the list
	MutexLocker mllist(&mutex_list);

	// if the list is empty, wait for a frame
	if(frames.size() == 0) {

		// wait for a new frame, or the end() function to signal
		pthread_cond_wait(&cond_list, &mutex_list);

		// check that we are still running...
		if(!running()) {

			// ...nope the end() -function woke us up, return NULL

			return NULL;

		}

	}

	/*
	 * at this point we can be sure that the list contains
	 * at least one frame. Return the oldest one and erase
	 * its pointer from the local list
	 */

	std::list<CameraFrame *>::iterator it = frames.begin();
	CameraFrame *ret = *it;
	frames.erase(it);

	return ret;

}


bool StreamWorker::isSpace() {

	// protect the list
	MutexLocker mllist(&mutex_list);

	return (int)frames.size() < max_n_frames;
}


size_t StreamWorker::getBufferState() {
	// protect the list
	MutexLocker mllist(&mutex_list);

	return frames.size();
}


bool StreamWorker::running() {

	MutexLocker mlrunning(&mutex_running);

	bool ret = b_running;

	return ret;

}


void StreamWorker::run() {

	/* loop while this thread is alive */
	while(running()) {

		// get the next frame from the list
		CameraFrame *frame = getNextFrame();

		// return to the beginning of the loop if the frame is NULL
		if(frame == NULL) {
			continue;
		}

		// process the frame
		CameraFrame *img_processed = process(frame);


		/*
		 * we must delete this image since the callback donated
		 * it to us. Delete only if the pointer is different
		 */
		if(frame != img_processed) {
			delete frame;
		}

		// return to the beginning of the loop if the processing was unsuccessfull
		if(img_processed == NULL) {
			continue;
		}

		// fire the callback
		bool cb_ret = cb_handler->frameProcessed(img_processed, user_data);

		// the return value states if the cb_handler is the owner or not
		if(!cb_ret) {
			// delete the frame because the cb handler did not take ownership
			delete img_processed;
		}

	}

}

