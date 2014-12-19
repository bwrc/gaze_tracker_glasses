#ifndef DUALCAMERARECEIVER_H
#define DUALCAMERARECEIVER_H


#include "CameraFrameExtended.h"
#include "GazeTracker.h"
#include "DataSink.h"
#include "GTWorker.h"
#include "ResultData.h"


class OutputData {

	public:
		OutputData();

		~OutputData();

		void releaseFrames();

		CameraFrameExtended **frames;
		ResultData *res;

};




/*
 *   ____________   ____________
 *   | camera 1 |   | camera 2 |
 *   ------------   ------------
 *        \              /
 *         \            /
 *       ___\__________/___
 *       | pair collector |
 *       ------------------
 *        |              |
 *        |both images   |eye image
 *    ____|_____     ____|_____
 *    | Socket |     | worker |
 *    | thread |<----| thread |
 *    ----------     ----------
 *
 */
class DualFrameReceiver : public FrameReceiver, public WorkerCBHandler {

	public:

		DualFrameReceiver();
		~DualFrameReceiver();

		/* Initialise everything. Must be called only once. */
		bool init(const std::string &socketFile,
				  const std::string &settings_file,
				  const std::string &sceneCamCalibFile,
				  const std::string &mapperFile);


		/* Inherited from FrameReceiver. Protected by a mutex. */
		void frameReceived(const CameraFrame *_frame, int id);

		/*
		 * Inherited from WorkerCBHandler, called by the workers and must
		 * therefore be mutexprotected.
		 */
		bool frameProcessed(CameraFrame *_frame, void *user_data);

		/* Return the current buffer queue sizes */
		size_t getWorkerBufferState();

		/* Return the maximum allowed worker buffer queue size */
		int getMaxBufferSize();

		/* Return the number of frames in the saver's queue */
		int getSaveBufferState();

	

		/* Return a pointer to the tracker */
		const gt::GazeTracker *getTracker() const {return tracker;}

	private:

		/* Create the gaze tracker */
		bool createTracker(const std::string &eyeCamCalibFile,
						   const std::string &sceneCamCalibFile,
						   const std::string &mapperFile);


		/* Configure the camera data. Called by createTracker() */
		bool configureEyeCamData(const std::string &eyeCamCalibFile);
		bool configureSceneCamData(const std::string &sceneCamCalibFile);
		bool configureMapper(const std::string &file);

		/* Join and dstroy the worker thread */
		void destroyGTWorker();


		/* A function that tells an instance of this class is alive */
		bool alive();


		/*
		 * Save the results to the output file. This function is called by
		 * frameProcessed(), which in turn is called from the GTWorker's thread.
		 */
		void saveResults(ResultData *res);


		/* Protect the class state */
		pthread_mutex_t mutex_alive;
		pthread_cond_t cond_alive;
		volatile bool b_alive;

		/* Decodes and analyses the frame */
		GTWorker *gtWorker;

		/* Sends frames and result to the client socket */
		DataSink dataSink;

		pthread_mutex_t mutex_receive;

		/* A container for both images */
		CameraFrameExtended *imgbuff_mjpg[2];

		/* The gaze tracker for one of the workers */
		gt::GazeTracker *tracker;

		/* Maps points to the scene */
		SceneMapper *mapper; 

		/* Camera for the tracker */
		Camera *camEye;

		/* Camera for the scene */
		Camera *camScene;

		/* Count the number of frame pairs received */
		unsigned long n_received_pairs;

		/* Tells if the workers are running, true if init was successful */
		volatile bool b_workerRunning;

};


#endif

