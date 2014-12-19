#include "Saver.h"

/* The number of frames, 2 cameras */
static const size_t NOF_FRAMES = 2;

/*
 * The maximum number of frame pairs stored in these:
 *     1. workers
 *     2. decoded images ready to be accessed by the user i.e. list_oput_frames
 */
static const size_t MAX_BUFFER_SIZE = 5;


Saver::Saver() {

	imgbuff_mjpg[0] = NULL;
	imgbuff_mjpg[1] = NULL;

	pthread_mutex_init(&mutex_receive, NULL);
	pthread_mutex_init(&mutex_oput, NULL);

	// zero the frame counts
	frame_counts[0] = frame_counts[1] = 0;

}


Saver::~Saver() {

	destroyWorkers();

	for(size_t i = 0; i < NOF_FRAMES; ++i) {
		delete oputstreams[i];
	}

	/* delete the output frames */
	pthread_mutex_lock(&mutex_oput);

		std::list<CameraFrame **>::iterator it = list_oput_frames.begin();
		while(it != list_oput_frames.end()) {

			CameraFrame **cur_frames = *it;
			CameraFrame *f1 = cur_frames[0];
			CameraFrame *f2 = cur_frames[1];

			delete f1;
			delete f2;
			delete[] cur_frames;

			++it;

		}

	pthread_mutex_unlock(&mutex_oput);

	pthread_mutex_destroy(&mutex_oput);


	/* delete the compressed frames */
	pthread_mutex_lock(&mutex_receive);

		delete imgbuff_mjpg[0];
		delete imgbuff_mjpg[1];

	pthread_mutex_unlock(&mutex_receive);

	pthread_mutex_destroy(&mutex_receive);

}


bool Saver::init(const std::vector<std::string> &save_paths) {

	// create the output streams
	if(!createOputStreams(save_paths)) {

		return false;

	}

	list_worker_user_data.resize(NOF_FRAMES);

	workers.resize(NOF_FRAMES);
	for(size_t i = 0; i < NOF_FRAMES; ++i) {

		// create the worker
		workers[i] = new JPEGWorker();

		// get a pointer to the current worker
		StreamWorker *cur = workers[i];

		int *user_data = new int;
		*user_data = i;

		list_worker_user_data[i] = user_data;

		// try initialising the worker
		if(!cur->init(this, MAX_BUFFER_SIZE, user_data)) {
			return false;
		}

		// start the worker thread
		if(!cur->start()) {
			return false;
		}

	}

	// we got this far, everything went ok
	return true;

}


/*
 * This function should execute as fast as possible.
 * All heavy operations must take place in separate
 * threads. JPEGWorkers are being used here.
 */
void Saver::frameReceived(const CameraFrame *_frame, int id) {

	// copy the frame
	CameraFrame *frame = new CameraFrame(*_frame);


	pthread_mutex_lock(&mutex_receive);

		// delete the the old frame if it is still in the buffer
		if(imgbuff_mjpg[id] != NULL) {
			delete imgbuff_mjpg[id];
		}

		// add to the buffer
		imgbuff_mjpg[id] = frame;

		// the index to the other frame
		const int ind_other = (id + 1) % NOF_FRAMES;

		// are both frames present?
		const bool b_both_present = imgbuff_mjpg[ind_other] != NULL;

		// if both images have been received
		if(b_both_present) {

			// increase the frame counts
			++frame_counts[0];
			++frame_counts[1];

			// first save the data
			for(size_t i = 0; i < NOF_FRAMES; ++i) {

				oputstreams[i]->write((const char *)imgbuff_mjpg[i]->data, imgbuff_mjpg[i]->sz);

			}

			/*
			 * See if there is space in both workers' queues. If either
			 * one is full, we will not add either frame to anywhere
			 */
			bool b_is_space = true;
			for(size_t i = 0; i < NOF_FRAMES; ++i) {

				b_is_space = workers[i]->isSpace();
				if(!b_is_space) {break;}

			}


			/*
			 * Only add the frames to the workers if both workers' queues allow
			 * for that.
			 */
			if(b_is_space) {

				// give the MJPG frames to the workers. They will place the decoded
				// frames into list_oput_frames
				for(size_t i = 0; i < NOF_FRAMES; ++i) {

					// add to worker
					workers[i]->add(imgbuff_mjpg[i]);

					// set to NULL, the worker handles memory management from now
					imgbuff_mjpg[i] = NULL;

				}

			}
			else {
				printf("One of the worker's queue is full, skipping\n");
			}

		}

	pthread_mutex_unlock(&mutex_receive);

}


bool Saver::frameProcessed(CameraFrame *_frame, void *user_data) {

	/* Drop the frame if the list is full */
	if(list_oput_frames.size() == MAX_BUFFER_SIZE) {

		printf("Saver::frameProcessed(): Dropping a frame. This might cause a syncing issue\n");

		// the caller can now release the memory of _frame
		return false;
	}

	int id = *((int *)user_data);

	// add the frame to the RGB buffer
	pthread_mutex_lock(&mutex_oput);

		CameraFrame **frame_pair = NULL;

		std::list<CameraFrame **>::iterator it = list_oput_frames.begin();
		while(it != list_oput_frames.end()) {

			CameraFrame **cur_frames = *it;
			
			if(cur_frames[id] == NULL) {

				cur_frames[id] = _frame;

				frame_pair = cur_frames;

				break;

			}

			++it;

		}

		if(frame_pair == NULL) {
			frame_pair = new CameraFrame*[NOF_FRAMES];
			frame_pair[id] = _frame;
			frame_pair[(id + 1) % NOF_FRAMES] = NULL;

			list_oput_frames.push_back(frame_pair);
		}

	pthread_mutex_unlock(&mutex_oput);


	// tell that we own this frame
	return true;

}


void Saver::destroyWorkers() {

	for(size_t i = 0; i < NOF_FRAMES; ++i) {

		// must be called to join the threads!
		workers[i]->end();

	}

	// delete after joining
	for(size_t i = 0; i < NOF_FRAMES; ++i) {

		delete workers[i];

	}

	/* destroy the userdata associated with the workers */
	for(size_t i = 0; i < NOF_FRAMES; ++i) {
		list_worker_user_data[i];
	}

}


bool Saver::getFrames(std::vector<CameraFrame *> &oput_rgb) {

	pthread_mutex_lock(&mutex_oput);

	// get the oldest frame pair
	std::list<CameraFrame **>::iterator it = list_oput_frames.begin();

	// see if the pair exists
	if(it != list_oput_frames.end()) {

		CameraFrame **frames = *it;

		// check if both images are present
		if(frames[0] != NULL && frames[1] != NULL) {

			// copy the pointers for the user
			oput_rgb[0] = frames[0];
			oput_rgb[1] = frames[1];

			// delete the frame pair container and remove it from the list
			delete[] frames;
			list_oput_frames.erase(it);

			pthread_mutex_unlock(&mutex_oput);
			return true;

		}
		else {
			pthread_mutex_unlock(&mutex_oput);
			return false;
		}

	}

	pthread_mutex_unlock(&mutex_oput);
	return false;

}


bool Saver::createOputStreams(const std::vector<std::string> &save_paths) {

	if(NOF_FRAMES != save_paths.size()) {
		return false;
	}

	for(size_t i = 0; i < NOF_FRAMES; ++i) {

		// if there is even 1 stream, that has not defined the save path,
		// no stream will be saved
		if(save_paths[i] == "null") {

			break;

		}

	}

	oputstreams.resize(NOF_FRAMES);

	std::vector<std::ofstream *>::iterator it = oputstreams.begin();

	while(it != oputstreams.end()) {

		int ind = it - oputstreams.begin();

		// try to create and open the output stream
		*it = new std::ofstream(save_paths[ind].c_str(), std::ofstream::binary);

		// check success
		if(!(*it)->is_open()) {

			// undo openings
			while(it >= oputstreams.begin()) {

				(*it)->close();
				--it;
			}

			return false;

		}

		++it;

	}


	return true;

}

