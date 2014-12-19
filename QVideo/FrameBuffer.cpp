#include "FrameBuffer.h"
#include <string.h>
#include <stdio.h>

#include <opencv2/imgproc/imgproc.hpp>

static const int WAIT_DUR = 200;
static const int BILLION = 1000000000;


static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;


void nsec2timespec(struct timespec *ts, uint64_t nanos) {
	ts->tv_sec	= nanos / (1e9);
	ts->tv_nsec	= nanos - ts->tv_sec * 1e9;
}


FrameBuffer::FrameBuffer() {
	memset(str_err, '\0', 1024);

	frames	= NULL;

	img_w = 0;
	img_h = 0;

	capacity		= 0;
	capacity_half	= 0;
	ind_put			= 0;
	ind_take		= 0;
	nof_frames		= 0;

	b_waiting		= false;
	b_running		= false;
}


FrameBuffer::~FrameBuffer() {
	stop();

	deInitBuffer();

	pthread_mutex_destroy(&mutex);

	pthread_cond_destroy(&cond);

}


bool FrameBuffer::create() {
	/*
	 *	Create the mutex
	 */

	if(pthread_mutex_init(&mutex, NULL) != 0) {
		sprintf(str_err, "%s", "could not initialise the mutex");
		return false;
	}


	/*
	 *	Create the condition variable
	 */

	if(pthread_cond_init(&cond, NULL) != 0) {
		sprintf(str_err, "%s", "could not initialise the condition variable");
		return false;
	}


	b_running = true;

	// on success, return true
	return true;
}


bool FrameBuffer::initBuffer(int _buff_capacity, int w, int h) {
	// empty the buffer
	deInitBuffer();

	pthread_mutex_lock(&mutex);

	// save the dimensions
	img_w = w;
	img_h = h;

	capacity		= 2*_buff_capacity;
	capacity_half	= _buff_capacity;

	frames = new cv::Mat[capacity];

	// allocate memory for the frames
	for(int i = 0; i < capacity; ++i) {
		frames[i] = cv::Mat(img_h, img_w, CV_8UC3);
	}

	pthread_mutex_unlock(&mutex);

	return true;
}


void FrameBuffer::deInitBuffer() {
	pthread_mutex_lock(&mutex);

	if(frames != NULL) {
		for(int i = 0; i < capacity; ++i) {
			frames[i].release();
		}

		delete[] frames;
		frames = NULL;
	}

	capacity		= 0;
	capacity_half	= 0;
	ind_take		= 0;
	ind_put			= 0;
	nof_frames		= 0;

	pthread_mutex_unlock(&mutex);
}


bool FrameBuffer::addFrame(const cv::Mat &frame) {
	pthread_mutex_lock(&mutex);

	// wait if the buffer is half full
	while(nof_frames >= capacity_half) {
		int ret = wait(WAIT_DUR);

		// if wait not interrupted by timeout or on signal, there has been an error
		if(ret != ETIMEDOUT && ret != 0) {

			sprintf(str_err, "%s", "addFrame() could not wait on condition");
			pthread_mutex_unlock(&mutex);
			return false;
		}

		if(!b_running) {
			sprintf(str_err, "%s", "addFrame() returned, because Streamer was ended");
			pthread_mutex_unlock(&mutex);
			return false;
		}
	}

	/*
	 *	convert the BGR-image into an RGB-image
	 */
	cv::cvtColor(frame, frames[ind_put], CV_BGR2RGB);

	// if the getFrame()-method is waiting, signal it
	if(b_waiting) {
		pthread_cond_signal(&cond);
	}

	// increment the number of available frames
	++nof_frames;

	// compute the index for the next put
	ind_put = (ind_put + 1) % capacity;

	pthread_mutex_unlock(&mutex);

	return true;
}


bool FrameBuffer::getFrame(cv::Mat *dest_rgb) {
	pthread_mutex_lock(&mutex);

	// wait if the buffer is empty
	while(nof_frames == 0) {

		int ret = wait(WAIT_DUR);

		if(ret != ETIMEDOUT && ret != 0) {
			sprintf(str_err, "%s%d", "getFrame() could not wait on condition: ", ret);
			pthread_mutex_unlock(&mutex);
			return false;
		}

		if(!b_running) {
			sprintf(str_err, "%s", "getFrame() returned, because Streamer was ended");
			pthread_mutex_unlock(&mutex);
			return false;
		}
	}

	if(dest_rgb->cols != img_w || dest_rgb->rows != img_h) {
		dest_rgb->release();
		*dest_rgb = cv::Mat(img_h, img_w, CV_8UC3);
	}


	frames[ind_take].copyTo(*dest_rgb);

	// decrement the number of available frames
	--nof_frames;

	// compute the index for the next take
	ind_take = (ind_take + 1) % capacity;

	// if the addFrame()-method is waiting, signal it
	if(b_waiting) {
		pthread_cond_signal(&cond);
	}

	pthread_mutex_unlock(&mutex);

	return true;
}


/*
 *	Skip n frames. n can also be negative. Returns true if the new position
 *	is within the buffer and false otherwise.
 */
bool FrameBuffer::skip(int n) {
	if(n == 0) {return true;}

	pthread_mutex_lock(&mutex);

	bool retVal = false;

	// the number of availabe slots in the buffer
	int nof_availabe = capacity_half - nof_frames;
	if(nof_availabe + n < capacity_half && nof_frames + n >= 0) {
		if(n > 0) {
			ind_take = (ind_take + n) % capacity;
		}
		else {
			ind_take = capacity + n + ind_take;
			ind_take = ind_take % capacity;
		}

		nof_frames -= n;
		retVal = true;
	}
	else {
		nof_frames	= 0;
		ind_put		= 0;
		ind_take	= 0;
	}

	pthread_mutex_unlock(&mutex);

	return retVal;
}


/*
 *	Note that the mutex must be locked before a call to this function
 */
int FrameBuffer::wait(int millis) {

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	ts.tv_sec	+= millis/1000;
	ts.tv_nsec	+= (millis % 1000) * 1000000;

	if(ts.tv_nsec >= BILLION) {
		ts.tv_sec++;
		ts.tv_nsec -= BILLION;
	}

	b_waiting = true;
	int ret = pthread_cond_timedwait(&cond, &mutex, &ts);
	b_waiting = false;

	return ret;
}


int FrameBuffer::getNofFrames() {
	pthread_mutex_lock(&mutex);
		int ret = nof_frames;
	pthread_mutex_unlock(&mutex);

	return ret;
}


int FrameBuffer::getCapacity() {
	pthread_mutex_lock(&mutex);
		int ret = capacity_half;
	pthread_mutex_unlock(&mutex);

	return ret;
}


void FrameBuffer::getError(char str_ret[1024]) {
	pthread_mutex_lock(&mutex);
		strcpy(str_ret, str_err);
	pthread_mutex_unlock(&mutex);
}


void FrameBuffer::stop() {
	pthread_mutex_lock(&mutex);
		b_running = false;

		if(b_waiting) {
			pthread_cond_broadcast(&cond);
		}
	pthread_mutex_unlock(&mutex);
}

