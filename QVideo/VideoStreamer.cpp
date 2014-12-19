#include "VideoStreamer.h"
#include <sys/time.h>
#include <vector>

#include <stdio.h>

static const int MAX_WAIT		= 200;
static const int MAX_BUFFER_LEN = 20;
static const int BILLION		= 1000000000;


bool isStreamerEnded(CAMERA_STATE *state);
bool isStreamerPaused(CAMERA_STATE *state);
void pauseStreamer(CAMERA_STATE *state);
void endStreamer(CAMERA_STATE *state);
void resumeStreamer(CAMERA_STATE *state);



VideoStreamer::VideoStreamer(StreamHandler *handler) {
	// store the handler
	this->handler = handler;
	this->handler->streamer = this;

	connect(this, SIGNAL(streamInit(STREAM_INIT_DATA *)),
			this->handler, SLOT(onStreamInit(STREAM_INIT_DATA *)), Qt::QueuedConnection);

	// initialise the VideoStreamer state
	memset(&this->cam_state, 0, sizeof(CAMERA_STATE));
	this->cam_state.mutex		= new QMutex();
	this->cam_state.isEnded		= false;	// has not yet ended
	this->cam_state.isPaused	= true;		// is paused initially
	this->cam_state.initOK		= false;	// not initialised yet
	this->cam_state.tot_frames	= 0;
	this->cam_state.frame_pos	= 0;

	// initialise the VideoStreamer settings
	this->settings = new SettingsStreamer();

	this->cam_state.mutex_sleeper = new QMutex();
	this->cam_state.cond_sleeper = new QWaitCondition();

	this->settings->setInput("null");
}


VideoStreamer::~VideoStreamer() {
	QMutexLocker enter(&this->handler->streamerLocker_mutex);

	this->capture.release();
	this->handler->streamer = NULL;

	delete this->cam_state.mutex_sleeper;
	delete this->cam_state.cond_sleeper;
	delete this->cam_state.mutex;

	delete this->settings;

	for(int i = 0; i < (int)list_init_data.size(); ++i) {
		delete list_init_data[i];
	}
}


/* The user launches this method. Remember that all things 
   owned by this thread, should be managed in this thread.
   So if the user calls this method, the calling thread is
   the GUI thread, and the resources cannot be altered. */
void VideoStreamer::startCapture() {
	resumeStreamer(&this->cam_state);

	this->cam_state.mutex_sleeper->lock();
		this->cam_state.cond_sleeper->wakeAll();
	this->cam_state.mutex_sleeper->unlock();
}


void VideoStreamer::pauseCapture() {
	pauseStreamer(&this->cam_state);
}


void VideoStreamer::stopCapture() {
	endStreamer(&this->cam_state);

	this->cam_state.mutex_sleeper->lock();
		this->cam_state.cond_sleeper->wakeAll();
	this->cam_state.mutex_sleeper->unlock();
}


/* On success this returns true, else it returns false */
bool VideoStreamer::readFrame() {
	// capture a frame, if unsuccessfull, pause
	if(!capture.grab()) {

		// drain the buffer
		// IMPLEMENT
printf("gargamel\n");
//		frames.deInitBuffer();

		pauseStreamer(&this->cam_state);

		capture.set(CV_CAP_PROP_POS_FRAMES, 0);

		return false;
	}

	// retrieve the captured frame
	capture.retrieve(img_bgr);

	return true;
}


bool VideoStreamer::init() {
	// everything is not ok unless the end of this function is reached
	this->cam_state.initOK = false;

	// release the current capture device if valid
	if(capture.isOpened()) {
		capture.release();
	}

	STREAM_INIT_DATA *init_data = new STREAM_INIT_DATA;
	memset(init_data, 0, sizeof(STREAM_INIT_DATA));
	list_init_data.push_back(init_data);


	// protect the camera state
	QMutexLocker locker(cam_state.mutex);

	// zero the frame position
	cam_state.frame_pos = 0;

	// the number of frames in the buffer
	int n = 0;

	// dimensions of a frame
	int w = 0;
	int h = 0;

	//-------------------- in case of camera stream --------------------
	char tmp[256];
	this->settings->getInput(tmp);

	if(memcmp(tmp, "null", 4) == 0) {
		return false;
	}
	else if(memcmp(tmp, "cam", 3) == 0) {
		int device = 0;

		if(strlen(tmp) == 4 && tmp[3] == '0') {
			device = 0;
		}
		else if(strlen(tmp) > 3) {
			device = atoi(tmp + 3);
		}

		// get the capture device
		capture.open(device);
		capture.set(CV_CAP_PROP_FRAME_WIDTH, settings->getWidth());
		capture.set(CV_CAP_PROP_FRAME_HEIGHT, settings->getHeight());

		if(capture.isOpened()) {

			this->settings->setFps(200);	// as fast as possible

			// number of frames in the buffer
			n = 1;

			/* First grab a frame and retrieve it so that we know the
			   image dimensions. */
			capture.grab();
			capture.retrieve(img_bgr);
			w = (int)this->img_bgr.cols;
			h = (int)this->img_bgr.rows;

			cam_state.tot_frames = 0;
			init_data->video_len = 0;
		}
	}

	//-------------------- in case of file stream --------------------
	else {

		// create the video capture device
		capture.open(tmp);

		if(capture.isOpened()) {

			this->settings->setFps((int)capture.get(CV_CAP_PROP_FPS));

			n = MAX_BUFFER_LEN;

			w = (int)capture.get(CV_CAP_PROP_FRAME_WIDTH);
			h = (int)capture.get(CV_CAP_PROP_FRAME_HEIGHT);

			cam_state.tot_frames = (unsigned int)capture.get(CV_CAP_PROP_FRAME_COUNT);
			init_data->video_len = cam_state.tot_frames;
		}
	}

	init_data->w = w;
	init_data->h = h;

	// see if the the initialisation went ok
	if(!capture.isOpened()) {
		this->cam_state.initOK = false;
		return false;
	}


	/* store the image dimensions into the settings struct */
	this->settings->setSize(w, h);

	// initialise the buffer
	frames.initBuffer(n, w, h);
	this->fillQueue();

	// everything went ok
	this->cam_state.initOK = true;


	// send the event to the GUI thread
	init_data->buffer_len = n;
	emit streamInit(init_data);

	return true;
}


void VideoStreamer::doStuffWithFrame(cv::Mat &img) {}


void VideoStreamer::fillQueue() {
	int capacity = frames.getCapacity();

	for(int i = 0; i < capacity; ++i) {
		this->readFrame();
		frames.addFrame(img_bgr);
	}
}


void VideoStreamer::run() {

	// create the frame buffer
	if(!frames.create()) {
		char str[1024];
		frames.getError(str);
		printf("%s\n", str);
		return;
	}

	// apply the settings
	this->applyChanges();

	// create the worker in this thread
	StreamWorker *worker = new StreamWorker(&(this->cam_state),
											&frames,
											this->settings,
											this);
	worker->start();

	QMutex mutexWorker;

	/*
	 *	The main loop
	 */
	while(!isStreamerEnded(&this->cam_state)) {

		/*
		 *	1. Apply possible changes requested by the user
		 */
		this->applyChanges();

		/*
		 *	2. Check if the streamer is paused
		 */
		if(!isStreamerPaused(&this->cam_state)) {

			/*
			 *	3. Get the next frame. Return to the beginning of the loop if not successfull
			 */
			if(!this->readFrame()) {
				continue;
			}

			/*	
			 *	4. Add the frame to the buffer
			 */
			if(!frames.addFrame(this->img_bgr)) {
				char str[1024];
				frames.getError(str);
				printf("FrameBuffer:\t%s\n", str);
				break;
			}

		}

		else {	// if streamer is paused

			/*
			 *	Put the streamer to sleep, and wait for the GUI to wake it up
			 */
			this->cam_state.mutex_sleeper->lock();

//				printf("streamer sleeps\n");
				this->cam_state.cond_sleeper->wait(this->cam_state.mutex_sleeper);
//				printf("streamer wakes\n");

			this->cam_state.mutex_sleeper->unlock();

		}
	}

	printf("streamer says bye\n");

	// stopping the buffer avoids the worker to wait on getFrame()
	frames.stop();


	/*
	 *	Destroy the worker thread
	 */
	{
		QMutexLocker locker(&mutexWorker);
		if(worker != NULL) {
			worker->wait();
			delete worker;
		}
	}

	// finally release the capture device
	if(capture.isOpened()) {
		capture.release();
	}

}


void VideoStreamer::doSetFrame(int new_pos) {
	cam_state.mutex->lock();

		// compute how much to skip in the buffer
		int skip = new_pos - cam_state.frame_pos;

		if(abs(skip) > 0 && new_pos < cam_state.tot_frames && new_pos >= 0) {

			// store the new position
			cam_state.frame_pos = new_pos;

	//		// get the number of frames in the buffer
	//		int nof_frames = frames.getNofFrames();

	//		// get the buffer capacity
	//		int capacity = frames.getCapacity();

			if(!frames.skip(skip)) {
				capture.set(CV_CAP_PROP_POS_FRAMES, new_pos);
			}
		}

	cam_state.mutex->unlock();
}


void VideoStreamer::doChangeInput() {
	if(!this->init()) {this->pauseCapture();}
}


void VideoStreamer::doChangeVideoSize() {
	if(!this->init()) {this->pauseCapture();}
}


/* user requested function */
void VideoStreamer::setFrame(unsigned int frame) {
	settings->setFrame(frame);
}


/* user requested function */
void VideoStreamer::setVideoDim(int w, int h) {
	settings->setSize(w, h);
}


/* 
 *	Skip n frames. n may be either positive or negative.
 */
void VideoStreamer::skipFrames(int n) {

}


void VideoStreamer::applyChanges() {
	unsigned char changes = this->settings->getChanges();
	if(changes) {
		// if the input has been changed
		if(changes & 0x08) {
			this->doChangeInput();
		}

		// if the frame pos has been changed
		if(changes & 0x10) {
			this->doSetFrame(settings->getFrame());
		}

		// if the frame video has been changed
		if(changes & 0x20) {
			this->doSetFrame(settings->getFrame());
		}

		// reset the changes
		this->settings->resetChanges();
	}
}



pthread_mutex_t mutexGUI = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condGUI = PTHREAD_COND_INITIALIZER;


/*
 *	This method is called by the message queue as a result of emit in the
 *	worker as a frame is ready to be drawn. The handler lives in the main
 *	thread as does this class, so a direct call to handler->onFrameChanged()
 *	is safe.
 */
void VideoStreamer::onFrameChanged(STREAM_FRAME_CHANGED_DATA *data) {

	if(data != NULL) {
		pthread_mutex_lock(&mutexGUI);




		if(!data->b_skip_frame) {
			/*
			 * Should be called before "handler->onFrameChanged(data)" so the worker's run knows that the drawing starts now
			 * Can be called here because the worker's run can access GUI things only after the mutex is released.
			 * However, should be called only if the frame is not requested to be skipped, because otherwise
			 * WaitGUI() thinks that this was a drawable frame and all went ok.
			 */
			pthread_cond_signal(&condGUI);

			handler->onFrameChanged(data);

		}

		pthread_mutex_unlock(&mutexGUI);

	}
}






StreamWorker::StreamWorker(	CAMERA_STATE *cam_state,
							FrameBuffer *_frames,
							SettingsStreamer *settings,
							VideoStreamer *streamer) {

	this->streamer	= streamer;
	this->settings	= settings;
	this->cam_state	= cam_state;

	this->frames	= _frames;

	if(pthread_mutex_init(&mutexGUI, NULL) != 0) {
		printf("%s", "could not initialise the mutex");
		return;
	}


	/*
	 *	Create the condition variable
	 */
	if(pthread_cond_init(&condGUI, NULL) != 0) {
		printf("%s", "could not initialise the condition variable");
		return;
	}


}


StreamWorker::~StreamWorker() {

	std::list<STREAM_FRAME_CHANGED_DATA *>::iterator it = vec_frame_data.begin();

	for(; it != vec_frame_data.end(); ++it) {
		delete (*it);
	}

	vec_frame_data.clear();


	pthread_mutex_destroy(&mutexGUI);

	pthread_cond_destroy(&condGUI);

}


/*
 *	Note that the mutex must be locked before a call to this function
 */
bool StreamWorker::waitGUI(int millis) {

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

	/*************************************************************************************************
	 * Wait on the condition
	 *************************************************************************************************/

	// on success returns zero
	int ret = pthread_cond_timedwait(&condGUI, &mutexGUI, &ts);


	return (bool)(ret == 0);	// ret == 0 means we were signalled
}


void StreamWorker::run() {
	struct timeval t1, t2;
	memset(&t1, 0, sizeof(struct timeval));
	memset(&t2, 0, sizeof(struct timeval));

	connect(this, SIGNAL(frameChanged(STREAM_FRAME_CHANGED_DATA *)),
			streamer, SLOT(onFrameChanged(STREAM_FRAME_CHANGED_DATA *)), Qt::QueuedConnection);


	/*
	 *	The main loop
	 */
	while(!isStreamerEnded(this->cam_state)) {

		/*
		 *	1. Check if the streamer is paused
		 */
		if(!isStreamerPaused(this->cam_state)) {

			// lock the GUI mutex
//			this->mutexGUI.lock();
			pthread_mutex_lock(&mutexGUI);


			/*
			 *	2. Get a frame from the buffer
			 */
			if(!frames->getFrame(&imgGUI)) {
				char str[1024];
				frames->getError(str);
				printf("FrameBuffer:\t%s\n", str);
//				this->mutexGUI.unlock();
				pthread_mutex_unlock(&mutexGUI);
				break;
			}

			// Do something with the frame
			this->streamer->doStuffWithFrame(imgGUI);


			cam_state->mutex->lock();
				int frame_pos = cam_state->frame_pos;
				++(cam_state->frame_pos);
			cam_state->mutex->unlock();


			/*
			 *	3. Send an event to the GUI
			 */
			STREAM_FRAME_CHANGED_DATA *data = new STREAM_FRAME_CHANGED_DATA;
//			memset(data, 0, sizeof(STREAM_FRAME_CHANGED_DATA));
			data->framePos		= frame_pos;
			data->buffState		= frames->getNofFrames();
//			data->cond			= &(this->condGUI);
//			data->mutex			= &(this->mutexGUI);
			data->cond			= &condGUI;
			data->mutex			= &mutexGUI;
			data->img_rgb		= &imgGUI;
			data->b_skip_frame	= false;

			/*
			 *	place the struct to the vector and delete on next successfull aknowledgement
			 *	or at the destructor
			 */
			vec_frame_data.push_back(data);

			// emit the message
			emit frameChanged(data);


			gettimeofday(&t2, NULL);


			/*
			 *	Computations to ensure the requested FPS
			 */
			int fps						= this->settings->getFps();		// FPS
			long dur_per_frame_ms		= (long)(1000.0 / fps + 0.5);	// frame duration in milliseconds
			long micros					= t2.tv_usec - t1.tv_usec;		// difference in microseconds
			long sec					= t2.tv_sec - t1.tv_sec;		// difference in seconds

			long dur_since_previous_ms	= (long)(1000.0 * sec + micros / 1000.0 + 0.5);	// duration since previous the frame
			long diff_ms				= dur_per_frame_ms - dur_since_previous_ms;		// difference in milliseconds


			// wait if untill it is time to process the frame in the GUI
			if(diff_ms > 1) {
				QThread::msleep(diff_ms);
			}


			// the time instant the GUI is allowed to paint
			gettimeofday(&t1, NULL);


			/*
			 * 4. Wait for the GUI to respond
			 */
//printf("wait...\n");
			//if(!this->condGUI.wait(&this->mutexGUI, MAX_WAIT)) {	// the GUI did not get the message
			if(!waitGUI(MAX_WAIT)) {
//printf("wait skipped\n");
				// this frame should be skipped by the frameChanged slot.
				data->b_skip_frame = true;

				// check the streamer state, quit if ended
				if(isStreamerEnded(this->cam_state)) {
//					this->mutexGUI.unlock();
					pthread_mutex_unlock(&mutexGUI);
					goto bye;
				}
				else {
					/*
					 *	Appears when the GUI is performing heavy processing on the frame, or when
					 *	the input is changed from file to camera. Might be due to different buffer sizes.
					 */
					printf("worker did not receive an acknowledgement\n");
				}
			}
			else {	// the GUI got the message

				/*
				 *	deallocate all messages.
				 */
				std::list<STREAM_FRAME_CHANGED_DATA *>::iterator it = vec_frame_data.begin();
				for(; it != vec_frame_data.end(); ++it) {
					delete (*it);
				}
				vec_frame_data.clear();
//printf("wait OK\n");
			}


			// unlock the GUI mutex
//			this->mutexGUI.unlock();
			pthread_mutex_unlock(&mutexGUI);
		}

		else {	// if(!isStreamerPaused(this->cam_state)) {

			/*
			 *	The streaming has been paused, so go to sleep.
			 *	The GUI-thread will wake this thread up, however, isStreamerPaused() might
			 *	return true, because applyChanges() has not yet been called. Therefore it
			 *	is important to timeout the wait(), so that we may continue streaming.
			 */

			this->cam_state->mutex_sleeper->lock();

				this->cam_state->cond_sleeper->wait(this->cam_state->mutex_sleeper, 100);

			this->cam_state->mutex_sleeper->unlock();

		}

	}	// the end of the main loop

	bye:

	printf("worker says bye\n");

	// stopping the buffer avoids the streamer to wait on addFrame()
	frames->stop();
}



/* This function is being used by both threads,
   therefore the mutex is used. */
bool isStreamerPaused(CAMERA_STATE *state) {
	bool ret = false;

	state->mutex->lock();
	if(state->isPaused) {
		ret = true;
	}
	else {
		ret = !state->initOK;
	}

	state->mutex->unlock();

	return ret;
}


/* This function is being used by both threads,
   therefore the mutex is used. */
bool isStreamerEnded(CAMERA_STATE *state) {
	state->mutex->lock();
		bool ret = state->isEnded;
	state->mutex->unlock();

	return ret;
}


/* This function is being used by both threads,
   therefore the mutex is used. */
void pauseStreamer(CAMERA_STATE *state) {
	state->mutex->lock();
		state->isPaused = true;
	state->mutex->unlock();
}


/* This function is being used by both threads,
   therefore the mutex is used. */
void endStreamer(CAMERA_STATE *state) {
	state->mutex->lock();
		state->isEnded = true;
	state->mutex->unlock();
}


/* This function is being used by both threads,
   therefore the mutex is used. */
void resumeStreamer(CAMERA_STATE *state) {
	state->mutex->lock();
		state->isPaused = false;
	state->mutex->unlock();
}




SettingsStreamer::SettingsStreamer() {
	this->mutex = new QMutex();
	QMutexLocker l(this->mutex);
	memset(&this->settings, '\0', sizeof(SETTINGS_STREAMER));
	sprintf(this->settings.fName, "%s", "null");
	this->requested_changes = 0;
}


SettingsStreamer::~SettingsStreamer() {
	delete this->mutex;
}


void SettingsStreamer::setFps(int fps) {
	QMutexLocker l(this->mutex);
	this->settings.fps = fps;
}


void SettingsStreamer::setSize(int w, int h) {
	QMutexLocker l(this->mutex);
	this->settings.width = w;
	this->settings.height = h;

	requested_changes |= 0x20;	// 0x10 = 00010 0000
}


void SettingsStreamer::setInput(const char *fName) {
	QMutexLocker l(this->mutex);
	this->requested_changes &= 0xf7;	// 0xf7 = 1111 0111
	this->requested_changes |= 0x08;	// 0x08 = 0000 1000
	strcpy(this->settings.fName, fName);
}


void SettingsStreamer::setFrame(int frame) {
	QMutexLocker l(this->mutex);
	requested_changes |= 0x10;	// 0x10 = 00001 0000
	settings.frame_pos = frame;
}


int SettingsStreamer::getFrame() {
	QMutexLocker l(this->mutex);
	return this->settings.frame_pos;
}


int SettingsStreamer::getFps() {
	QMutexLocker l(this->mutex);
	return this->settings.fps;
}


void SettingsStreamer::getSize(int *w, int *h) {
	QMutexLocker l(this->mutex);
	*w = this->settings.width;
	*h = this->settings.height;
}


int SettingsStreamer::getWidth() {
	QMutexLocker l(this->mutex);

	return this->settings.width;
}


int SettingsStreamer::getHeight() {
	QMutexLocker l(this->mutex);

	return this->settings.height;
}


void SettingsStreamer::getInput(char *fName) {
	QMutexLocker l(this->mutex);
	strcpy(fName, this->settings.fName);
}


void SettingsStreamer::resetChanges() {
	QMutexLocker l(this->mutex);
	this->requested_changes = 0x00;
}


unsigned char SettingsStreamer::getChanges() {
	QMutexLocker l(this->mutex);
	return this->requested_changes;
}

