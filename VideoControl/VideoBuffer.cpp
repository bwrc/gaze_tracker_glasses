#include <list>
#include <gst/gst.h>
#include <gst/video/video.h>
#include "VideoBuffer.h"
#include <iostream>



MultiImage::MultiImage(size_t sz) {

	// images are null and we are not ready
	imgs.resize(sz);
	b_ready = false;

	std::vector<CameraFrame *>::iterator it = imgs.begin();

	while(it != imgs.end()) {
		*it = NULL;
		++it;
	}

}


void MultiImage::release() {

	std::vector<CameraFrame *>::iterator it = imgs.begin();

	while(it != imgs.end()) {
		delete *it;
		*it = NULL;
		++it;
	}

}



MultiImageBuffer::MultiImageBuffer(int _max_buff_size, size_t _nimages) {
    pthread_mutex_init(&mutex, NULL);
	nimages = _nimages;
	max_buff_size = _max_buff_size;

	b_wait_for_others = false;

}



MultiImageBuffer::~MultiImageBuffer() {

	pthread_mutex_lock(&mutex);

		std::list<MultiImage>::iterator it = images.begin();

		while(it != images.end()) {
			it->release();

			++it;
		}

		images.clear();

	pthread_mutex_unlock(&mutex);

   	pthread_mutex_destroy(&mutex);

}




/*
 * Add the image to the list. The flowchart below shows how it works for two images.
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
void MultiImageBuffer::add(CameraFrame *img, int image_type) {

	// locks the mutex in the constructor and releases in the destructor
	MutexLocker mutex_locker(&mutex);


	/***************************************************
	 * see if the other images have been arrived
	 **************************************************/
	if(b_wait_for_others) { // waiting for a other images

		// points to a valid element, since at least one image is
		// already waiting for the others
		std::list<MultiImage>::iterator ite = images.end();
		--ite;

		std::vector<CameraFrame *> &imgs = ite->imgs;

		/***************************************************
		 * see if this slot is vacant
		 **************************************************/
		bool b_is_vacant = imgs[image_type] == NULL;

		if(b_is_vacant) { // is vacant

			imgs[image_type] = img;

		}
		else { // is not vacant

			delete imgs[image_type];
			imgs[image_type] = img;

		}

	}

	else { // not waiting for other images

		// see if full
		if((int)images.size() == max_buff_size) {

			// clear resources from the oldest and remove it from the list
			std::list<MultiImage>::iterator itf = images.begin();
			itf->release();

			images.erase(itf);
std::cout << "VideoBuffer: the buffer is full, removing the oldest and inserting..." << std::endl;

		}

		// insert new
		MultiImage ip(nimages);
		ip.imgs[image_type] = img;
		images.push_back(ip);

	}


	// points to a valid element, since at least one image is waiting for the others
	std::list<MultiImage>::iterator ite = images.end();
	--ite;

	// see if all images have been placed
	std::vector<CameraFrame *> &imgs = ite->imgs;
	std::vector<CameraFrame *>::iterator imgit = imgs.begin();
	b_wait_for_others = false;
	while(imgit < imgs.end()) {

		if(*imgit == NULL) {
			b_wait_for_others = true;
			break;
		}

		++imgit;

	}

	ite->b_ready = !b_wait_for_others;

}


/*
 * oput must be of the right size
 *
 * IMPORTANT: the caller must delete the images on success.
 */
bool MultiImageBuffer::getFrames(std::vector<CameraFrame *> &oput) {

	MutexLocker locker(&mutex);

	if(images.size() == 0) {
		return false;
	}


	// get the oldest image group
	std::list<MultiImage>::iterator itf = images.begin();

	if(itf->b_ready) {

		std::vector<CameraFrame *> &imgs = itf->imgs;
		std::vector<CameraFrame *>::iterator itimgs = imgs.begin();

		while(itimgs != imgs.end()) {

			int ind = itimgs - imgs.begin();
			oput[ind] = imgs[ind];

			++itimgs;

		}

		// now remove the element from the list
		images.erase(itf);

		return true;

	}


	return false;

}

