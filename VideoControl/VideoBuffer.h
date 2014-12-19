#ifndef VIDEO_BUFFER_H
#define VIDEO_BUFFER_H


#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <list>
#include <pthread.h>
#include <gst/gst.h>
#include <gst/video/video.h>
#include "CaptureDevice.h"



/*
 * A convenient class for automatic locking and unlocking of a given mutex
 */
class MutexLocker {

	public:

		MutexLocker(pthread_mutex_t *_mutex) {
			mutex = _mutex;
			pthread_mutex_lock(mutex);
		}

		~MutexLocker() {
			pthread_mutex_unlock(mutex);
		}

	private:
		pthread_mutex_t *mutex;

};


/*
 * A container for storing multiple images. Used by MultiImageBuffer
 */
class MultiImage {

	public:

		MultiImage(size_t sz);

		/*
		 * Delete contents of the images
		 */
		void release();

		/*
		 * Images
		 */
		std::vector<CameraFrame *> imgs;

		/* true when all images are present, false otherwise. Toggled by MultiImageBuffer */
		bool b_ready;

};



/*
 * This class stores multiple images from multiple streams sent by
 * gstreamer through cb().
 */
class MultiImageBuffer {

	public:

		/*
		 * nimages: Number of images to be put into one element
		 */
		MultiImageBuffer(int _max_buff_size, size_t nimages);

		~MultiImageBuffer();


		/*
		 * Add the image to the list. The flowchart below shows how it works for
		 * two images.
		 *
		 *                                __________________
		 *                               |    Image in      |
		 *                                ------------------
		 *
		 *           __________________                        ________________________
		 *          | waiting for pair |                      | not waiting for a pair |
		 *           ------------------                        ------------------------
		 *
		 *   _________          _____________              _________                  _____________
		 *  | is pair |        | is not pair |            | is full |                | is not full |
		 *   ---------          -------------              ---------                  -------------
		 *
		 *   ________             __________        ____________________________     ______________________
		 *  | insert |           | replace |       | Release oldest, insert new |   | insert a new element |
		 *   --------             ---------         ----------------------------     ----------------------
		 *
		 */
		void add(CameraFrame *img, int image_type);

		/*
		 * The oput vector must be of the correct size.
		 * Will not block, so if there is no data, false will be returned,
		 * otherwise true will be returned.
		 *
		 * IMPORTANT: the caller must delete the images on success.
		 */
		bool getFrames(std::vector<CameraFrame *> &oput);

		/* Get the maximum allowed size of the buffer */
		int getMaxSize() {return max_buff_size;}

	private:

		/* List of image groups */
		std::list<MultiImage> images;

		/* Protects the images list */
		pthread_mutex_t mutex;

		/* The number of images in a group */
		size_t nimages;

		/* The maximum size of the buffer */
		int max_buff_size;

		bool b_wait_for_others;

};



#endif

