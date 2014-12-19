#ifndef SAVER_H
#define SAVER_H


#include "VideoBuffer.h"
#include "CameraFrame.h"
#include "VideoHandler.h"



class Saver : public FrameReceiver, public WorkerCBHandler {

	public:

		Saver();
		~Saver();

		bool init(const std::vector<std::string> &save_paths);


		/* Inherited from FrameReceiver. Protected by a mutex. */
		void frameReceived(const CameraFrame *_frame, int id);

		/*
		 * Get the frames, succeeds if both frames are ready.
		 * Important! The caller must release the images with "delete".
		 */
		bool getFrames(std::vector<CameraFrame *> &oput_rgb);


		/*
		 * Inherited from WorkerCBHandler, called by the workers and must
		 * therefore be mutexprotected.
		 */
		bool frameProcessed(CameraFrame *_frame, void *user_data);

	private:

		/* Create the output streams if required */
		bool createOputStreams(const std::vector<std::string> &save_paths);


		/* Join and dstroy the worker threads */
		void destroyWorkers();

		/* for each stream there is a worker to decode the data etc. */
		std::vector<StreamWorker *> workers;

		/* User defined data for the workers, stored in this vector */
		std::vector<int *> list_worker_user_data;

		/* Output files */
		std::vector<std::ofstream *> oputstreams;

		pthread_mutex_t mutex_receive;

		/* A container for both images */
		CameraFrame *imgbuff_mjpg[2];


		/* A mutex protecting the output frames */
		pthread_mutex_t mutex_oput;

		/* List of output frame pairs */
		std::list<CameraFrame **> list_oput_frames;


		/* Frame counts for both frames */
		unsigned long frame_counts[2];

};


#endif

