#include "DualFrameReceiver.h"
#include "trackerSettings.h"
#include "CalibDataReader.h"
#include "localTrackerSettings.h"
#include "MapperReader.h"

#include <stdio.h>
#include <time.h>


static const int DBL_ACCURACY = 5;


// global
pthread_mutex_t mutex_tracker;



/* The number of frames, 2 cameras */
static const size_t NOF_FRAMES = 2;

/*
 * The maximum number of frame pairs stored in the GTWorker
 */
static const size_t MAX_BUFFER_SIZE = 100;


OutputData::OutputData() {
	frames = new CameraFrameExtended*[NOF_FRAMES];
	frames[0] = frames[1] = NULL;

	res = NULL;
}


OutputData::~OutputData() {
//	delete frames[0];
//	delete frames[1];
	delete[] frames;

	delete res;
}


void OutputData::releaseFrames() {
	delete frames[0];
	delete frames[1];
}



DualFrameReceiver::DualFrameReceiver() {

	b_workerRunning = false;

	imgbuff_mjpg[0] = NULL;
	imgbuff_mjpg[1] = NULL;


	b_alive = false;

	mapper		= NULL;
	tracker		= NULL;
	camEye		= NULL;
	camScene	= NULL;

}


DualFrameReceiver::~DualFrameReceiver() {

	/***************************************************
	 * State that this instance is no longer alive
	 ***************************************************/
	pthread_mutex_lock(&mutex_alive);
		b_alive = false;
	pthread_mutex_unlock(&mutex_alive);


	/***************************************************
	 * Destroy the GTWorker
	 ***************************************************/
	if(b_workerRunning) {

		destroyGTWorker();

	}


	/***************************************************
	 * Join the writer thread
	 ***************************************************/
	dataSink.end();


	/************************************************
	 * Delete the tracker-related stuff
	 ************************************************/

	// delete the cameras
	delete camEye;
	delete camScene;


	// delete the gaze tracker
	delete tracker;

	// delete the scene mapper
	delete mapper;


	/* delete the compressed frames */
	pthread_mutex_lock(&mutex_receive);

		delete imgbuff_mjpg[0];
		delete imgbuff_mjpg[1];

	pthread_mutex_unlock(&mutex_receive);


	/*
	 * Destroy these only after destroyGTWorker(), because frameProcessed()
	 * calls alive().
	 */
	pthread_mutex_destroy(&mutex_alive);
	pthread_cond_destroy(&cond_alive);


	pthread_mutex_destroy(&mutex_tracker);

	pthread_mutex_destroy(&mutex_receive);

}


bool DualFrameReceiver::init(const std::string &socketFile,
							 const std::string &eyeCamCalibFile,
							 const std::string &sceneCamCalibFile,
							 const std::string &mapperFile) {


	/***********************************************
	 * Create mutexes and conditions
	 ***********************************************/
	int ret;
	ret = pthread_mutex_init(&mutex_tracker, NULL);
	if(ret != 0) {
		printf("DualFrameReceiver::init(): Could not create the mutex for the tracker\n");
		return false;
	}

	ret = pthread_mutex_init(&mutex_receive, NULL);
	if(ret != 0) {
		printf("DualFrameReceiver::init(): Could not create the mutex receiver mutex\n");
		return false;
	}

	ret = pthread_mutex_init(&mutex_alive, NULL);
	if(ret != 0) {
		printf("DualFrameReceiver::init(): Could not create the mutex for the alive state\n");
		return false;
	}

	ret = pthread_cond_init(&cond_alive, NULL);
	if(ret != 0) {
		printf("DualFrameReceiver::init(): Could not create the condition variable for the alive state\n");
		return false;
	}


	// zero frame pairs received so far
	n_received_pairs = 0;

	/*************************************************************
	 * Create the client socket
	 *************************************************************/
	if(!dataSink.init(socketFile.c_str())) {

		return false;

	}


	/*************************************************************
	 * Create the tracker
	 *************************************************************/
	if(!createTracker(eyeCamCalibFile, sceneCamCalibFile, mapperFile)) {

		return false;
	}


	/*************************************************************
	 * Create the GTWorker
	 *************************************************************/

	// create the worker
	gtWorker = new GTWorker();
	gtWorker->setTracker(tracker, mapper);

	// try initialising the worker
	if(!gtWorker->init(this, MAX_BUFFER_SIZE, NULL)) {
		return false;
	}

	// start the worker
	if(!gtWorker->start()) {
		return false;
	}


	// if we got this far the worker is running
	b_workerRunning = true;

	// ...and the instance is alive
	b_alive = true;

	// ...and everything went ok
	return true;

}


void DualFrameReceiver::destroyGTWorker() {

	printf("DualFrameReceiver::destroyGTWorker(): Destroy worker...\n");

	// must be called to join the threads!
	gtWorker->end();
	printf("  end worker\n");

	printf("  delete worker\n");
	delete gtWorker;

}


/*
 * This function should execute as fast as possible.
 * All heavy operations must take place in separate
 * threads. A JPEGWorker and a GTWorker are being used here.
 */
void DualFrameReceiver::frameReceived(const CameraFrame *_frame, int id) {

	// copy the frame
	CameraFrameExtended *frame = new CameraFrameExtended(*_frame);

	pthread_mutex_lock(&mutex_receive);

		// delete the the old frame if it is still in the buffer
		if(imgbuff_mjpg[id] != NULL) {
			delete imgbuff_mjpg[id];
			printf("Camera %d is slow\n", (int)((id+1) % NOF_FRAMES));
		}

		// add to the buffer
		imgbuff_mjpg[id] = frame;

		// the index to the other frame
		const int ind_other = (id + 1) % NOF_FRAMES;

		// are both frames present?
		const bool b_both_present = imgbuff_mjpg[ind_other] != NULL;


		// if both images have been received
		if(b_both_present) {

			imgbuff_mjpg[0]->id = n_received_pairs;
			imgbuff_mjpg[1]->id = n_received_pairs;

			// increment the frame pair counter
			++n_received_pairs;


			// first copy the data to the sink
			dataSink.addFrames(imgbuff_mjpg);


			/*
			 * See if there is space in the worker's queue. If it is
			 * full, we will not add the frame to the worker
			 */
			const bool b_is_space = gtWorker->isSpace();

			/*
			 * Only add the frames to the worker if the worker's queue allows
			 * for that.
			 */
			if(b_is_space) {


				// add to worker
				gtWorker->add(imgbuff_mjpg[0]);

				// set to NULL, the worker handles memory management from now
				imgbuff_mjpg[0] = NULL;

				// ...however, delete the scene frame
				delete imgbuff_mjpg[1];
				imgbuff_mjpg[1] = NULL;

			}
			else {

				/*
				 * Delete the frames, because they could not be
				 * donated to the worker
				 */

				delete imgbuff_mjpg[0];
				delete imgbuff_mjpg[1];

				imgbuff_mjpg[0] = imgbuff_mjpg[1] = NULL;

				printf("DualFrameReceiver::frameReceived(): The worker's queue is full, skipping\n");

			}

		}


	pthread_mutex_unlock(&mutex_receive);

}


bool DualFrameReceiver::frameProcessed(CameraFrame *_frame, void *user_data) {

	CameraFrameExtended *frameExtended = (CameraFrameExtended *)_frame;

	// write the results
	saveResults(frameExtended->res);

	// UGLY
	delete frameExtended->res;
	delete frameExtended;


	// tell that we own this frame
	return true;

}


void DualFrameReceiver::saveResults(ResultData *res) {

//	std::vector<char> buff;
//	BinaryResultParser::resDataToBuffer(*res, buff);

//	// insert into file
//	dataSink.add(buff.data(),
//				 buff.size(),
//				 DataContainer::TYPE_TRACK_RESULTS);

	dataSink.addResults(res);

}


size_t DualFrameReceiver::getWorkerBufferState() {
	return gtWorker->getBufferState();
}


int DualFrameReceiver::getMaxBufferSize() {
	return MAX_BUFFER_SIZE;
}


int DualFrameReceiver::getSaveBufferState() {

	return dataSink.getBufferState();

}


bool DualFrameReceiver::alive() {

	pthread_mutex_lock(&mutex_alive);
		bool ret = b_alive;
	pthread_mutex_unlock(&mutex_alive);

	return ret;
}


bool DualFrameReceiver::configureEyeCamData(const std::string &eyeCamCalibFile) {

	/**********************************************************************
	 * Open the settings file and read the settings of the calib reader
	 **********************************************************************/
	CalibDataReader calibReader;
	if(!calibReader.create(eyeCamCalibFile)) {

		printf("DualFrameReceiver::configureEyeCamData(): Could not load \"%s\"\n",
				eyeCamCalibFile.c_str());

		return false;

	}


	/**********************************************************************
	 * Read the camera calibration container and assign the values to
	 * the camera
	 **********************************************************************/
	{

		calib::CameraCalibContainer camContainer;
		if(!calibReader.readCameraContainer(camContainer)) {

			printf("DualFrameReceiver::configureEyeCamData(): Could not read the camera container\n");

			return false;

		}


		/**********************************************************************
		 * Create the eye camera
		 **********************************************************************/

		// create the camera
		camEye = new Camera();
		if(camEye == NULL) {
			return false;
		}

		// set the parameters
		camEye->setIntrinsicMatrix(camContainer.intr);
		camEye->setDistortion(camContainer.dist);

	}


	/**********************************************************************
	 * Load the LED positions from the file and assign the to the pattern
	 *********************************************************************/

    std::vector<calib::LEDCalibContainer> LEDContainers;
    if(!calibReader.readLEDContainers(LEDContainers)) {

        printf("DualFrameReceiver::configureEyeCamData(): Could not read the LED containers\n");

        return false;

    }

    int nLEDs = (int)LEDContainers.size();
    if(nLEDs != 6) {

        printf(	"DualFrameReceiver::configureEyeCamData():"
                "Erroneous configuration file \"%s\":"
                "Unsupported LED configuration\n", eyeCamCalibFile.c_str());

        return false;

    }

    std::vector<cv::Point3d> positions(nLEDs);

    // Go through each of the LEDs
    for(int i = 0; i < nLEDs; ++i) {

        // get the current container
        const calib::LEDCalibContainer &curLEDContainer = LEDContainers[i];

        double x = curLEDContainer.LED_pos[0];
        double y = curLEDContainer.LED_pos[1];
        double z = curLEDContainer.LED_pos[2];

        // .. store it
        positions[i] = cv::Point3d(x, y, z);

    }


	/**********************************************************************
	 * Create and initialise the tracker
	 *********************************************************************/
	tracker = new gt::GazeTracker();
	tracker->init(*camEye, positions);

	return true;

}


bool DualFrameReceiver::configureSceneCamData(const std::string &sceneCamCalibFile) {

	/**********************************************************************
	 * Open the settings file and read the settings of the calib reader
	 **********************************************************************/
	CalibDataReader calibReader;
	if(!calibReader.create(sceneCamCalibFile)) {

		printf("DualFrameReceiver::configureSceneCamData(): Could not load \"%s\"\n",
				sceneCamCalibFile.c_str());

		return false;

	}


	/**********************************************************************
	 * Read the camera calibration container and assign the values to
	 * the camera
	 **********************************************************************/
	calib::CameraCalibContainer camContainer;
	if(!calibReader.readCameraContainer(camContainer)) {

		printf("DualFrameReceiver::configureSceneCamData(): Could not read the camera container\n");

		return false;

	}


	/**********************************************************************
	 * Create and initialise the camera
	 **********************************************************************/
	camScene = new Camera();
	camScene->setIntrinsicMatrix(camContainer.intr);
	camScene->setDistortion(camContainer.dist);


	return true;

}


bool DualFrameReceiver::configureMapper(const std::string &file) {

	/*********************************************************************
	 * Create and initialise the mapper
	 *********************************************************************/

	MapperReader rder;
	if(rder.readContents(file)) {

		const cv::Mat &tr = rder.getTransformation();

		Eigen::Matrix4d A;
		A.row(0) << tr.at<double>(0, 0), tr.at<double>(0, 1), tr.at<double>(0, 2), tr.at<double>(0, 3);
		A.row(1) << tr.at<double>(1, 0), tr.at<double>(1, 1), tr.at<double>(1, 2), tr.at<double>(1, 3);
		A.row(2) << tr.at<double>(2, 0), tr.at<double>(2, 1), tr.at<double>(2, 2), tr.at<double>(2, 3);
		A.row(3) << tr.at<double>(3, 0), tr.at<double>(3, 1), tr.at<double>(3, 2), tr.at<double>(3, 3);

		mapper = new SceneMapper(A, camScene);

		return true;

	}

	printf("DualFrameReceiver::configureMapper(): could not read contents of %s\n", file.c_str());

	return false;

}


bool DualFrameReceiver::createTracker(const std::string &eyeCamCalibFile,
									  const std::string &sceneCamCalibFile,
									  const std::string &mapperFile) {

	if(!configureEyeCamData(eyeCamCalibFile)) {
		return false;
	}

	if(!configureSceneCamData(sceneCamCalibFile)) {
		return false;
	}

	if(!configureMapper(mapperFile)) {
		return false;
	}

	return true;

}

