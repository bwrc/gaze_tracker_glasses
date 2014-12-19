#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H


#include <pthread.h>
//#include <opencv/cv.h>
#include <opencv2/highgui/highgui.hpp>

#include <sys/time.h>
#include <time.h>
#include <errno.h>


/*
 *	This class implements buffering of frames. Frames can be added until the
 *	maximum amount is reched and frames can be retrieved as long as there is
 *	at least one frame in the buffer
 */
class FrameBuffer {
	public:
		FrameBuffer();
		~FrameBuffer();


		/*
		 *	Must be called always before calling any other method
		 */
		bool create();

		void stop();

		/*
		 *	Allocate memory for the frames and reset the variables
		 */
		bool initBuffer(int nof_frames, int w, int h);

		/*
		 *	Deallocate memory from the frames and reset the variables
		 */
		void deInitBuffer();

		/*
		 *	Adds a frame into the buffer. Blocks if the buffer is full
		 */
		bool addFrame(const cv::Mat &frame);

		/*
		 *	Skip n frames. n can also be negative. Returns true if the new position
		 *	is within the buffer and false otherwise.
		 */
		bool skip(int n);

		/*
		 *	Get a frame from the buffer. Blocks if the buffer is empty
		 */
		bool getFrame(cv::Mat *dest);

		int getNofFrames();

		int getCapacity();

		void getError(char str_ret[1024]);

	private:
		int wait(int millis);

		cv::Mat *frames;

		// image dimensions
		int img_w, img_h;

		// number of frames that fit into the buffer
		int capacity;

		// half of the capacity
		int capacity_half;

		// indices that show where to put and get the next frames
		int ind_put, ind_take;

		// number of available frames
		int nof_frames;

//		pthread_mutex_t *mutex;
//		pthread_cond_t *cond;

		bool b_waiting;
		bool b_running;

		char str_err[1024];
};


#endif

