#include "DualFrameReceiver.h"
#include "trackerSettings.h"
#include "CalibDataReader.h"
#include "localTrackerSettings.h"
#include "BinaryResultParser.h"
#include "MapperReader.h"
#include "SceneFrameWorker.h"
#include "ResultWriter.h"

#include <stdio.h>
#include <time.h>


// global
pthread_mutex_t mutex_tracker;



/* The number of frames, 2 cameras */
static const size_t NOF_FRAMES = 2;

/*
 * The maximum number of frame pairs stored in these:
 *     1. workers
 *     2. decoded images ready to be accessed by the user i.e. list_oput
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



DualFrameReceiver::DualFrameReceiver(bool _bGUIActive) {

	b_workers_running = false;

	b_alive = false;

	mapper		= NULL;
	tracker		= NULL;
	camEye		= NULL;
	camScene	= NULL;

	bGUIActive = _bGUIActive;

}


DualFrameReceiver::~DualFrameReceiver() {

	/***************************************************
	 * State that this instance is no longer alive
	 ***************************************************/
	pthread_mutex_lock(&mutex_alive);
		b_alive = false;
	pthread_mutex_unlock(&mutex_alive);


	/***************************************************
	 * Destroy the output frames
	 ***************************************************/
	pthread_mutex_lock(&mutex_oput);
	flushGUIQueue();


	/*******************************************************
	 * Broadcast instead of Signal, because both workers
	 * might be hanging in frameProcessed()
	 *******************************************************/
	pthread_cond_broadcast(&cond_oput);
	pthread_mutex_unlock(&mutex_oput);


	/***************************************************
	 * Destroy the workers
	 ***************************************************/
	if(b_workers_running) {

		destroyWorkers();

		/*
		 * At least one worker might have launched frameProcessed() during
		 * the call to destroyWorkers(), so send a broadcast
		 */
		pthread_mutex_lock(&mutex_oput);
		pthread_cond_broadcast(&cond_oput);
		pthread_mutex_unlock(&mutex_oput);

	}


	/***************************************************
	 * Join the writer thread and delete it
	 ***************************************************/
	video_writer->end();

    delete video_writer;


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


	pthread_mutex_destroy(&mutex_oput);
	pthread_cond_destroy(&cond_oput);


	/*
	 * Destroy these only after destroyWorkers(), because frameProcessed()
	 * calls alive().
	 */
	pthread_mutex_destroy(&mutex_alive);
	pthread_cond_destroy(&cond_alive);


	pthread_mutex_destroy(&mutex_tracker);

	pthread_mutex_destroy(&mutex_receive);

}


bool DualFrameReceiver::init(bool bOnlyResults,
                             const std::string &saveDir,
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

	ret = pthread_mutex_init(&mutex_oput, NULL);
	if(ret != 0) {
		printf("DualFrameReceiver::init(): Could not create the mutex for output list\n");
		return false;
	}

	ret = pthread_mutex_init(&mutex_alive, NULL);
	if(ret != 0) {
		printf("DualFrameReceiver::init(): Could not create the mutex for the alive state\n");
		return false;
	}

	ret = pthread_cond_init(&cond_oput, NULL);
	if(ret != 0) {
		printf("DualFrameReceiver::init(): Could not create the condition variable for the output list\n");
		return false;
	}

	ret = pthread_cond_init(&cond_alive, NULL);
	if(ret != 0) {
		printf("DualFrameReceiver::init(): Could not create the condition variable for the alive state\n");
		return false;
	}


	// zero frame pairs received so far
	n_received_pairs = 0;

    if(saveDir == "null") {
        video_writer = new DevNullWriter();
    }
    else if(bOnlyResults) {
        video_writer = new ResultWriter();
    }
    else {
        video_writer = new VideoWriter();
    }


	/*************************************************************
	 * Create the output streams
	 *************************************************************/
	if(!video_writer->init(saveDir)) {
		return false;
	}
	if(!video_writer->start()) {
		return false;
	}



	/*************************************************************
	 * Create the tracker
	 *************************************************************/
	if(!createTracker(eyeCamCalibFile, sceneCamCalibFile, mapperFile)) {
		return false;
	}


	/*************************************************************
	 * Create the workers
	 *************************************************************/
	list_worker_user_data.resize(NOF_FRAMES);

	workers.resize(NOF_FRAMES);
	for(size_t i = 0; i < NOF_FRAMES; ++i) {

		// create the worker
		if(i == 1) {
			workers[i] = new SceneFrameWorker();
		}
		else {
			GTWorker *w = new GTWorker();
			w->setTracker(tracker, mapper);
			workers[i] = w;
		}

		// get a pointer to the current worker
		StreamWorker *cur = workers[i];

		// define the user data, which is the index of this worker in the worker's list
		int *user_data = new int;
		*user_data = i;

		list_worker_user_data[i] = user_data;

		// try initialising the worker
		if(!cur->init(this, MAX_BUFFER_SIZE, user_data)) {
			return false;
		}

	}

	// start the workers
	if(!workers[0]->start()) {
		return false;
	}

	if(!workers[1]->start()) {
		return false;
	}


	// if we got this far the workers are running
	b_workers_running = true;

	// ...and the instance is alive
	b_alive = true;

	// ...and everything went ok
	return true;

}


void DualFrameReceiver::destroyWorkers() {

	printf("DualFrameReceiver::destroyWorkers(): Destroy workers...\n");

	for(size_t i = 0; i < NOF_FRAMES; ++i) {

		// must be called to join the threads!
		workers[i]->end();

		printf("  end worker %d\n", (int)i);

	}

	// delete after joining
	for(size_t i = 0; i < NOF_FRAMES; ++i) {

		printf("  delete worker %d\n", (int)i);

		delete workers[i];

	}

	/* destroy the userdata associated with the workers */
	for(size_t i = 0; i < NOF_FRAMES; ++i) {
		delete list_worker_user_data[i];
	}

}


/*
 * This function should execute as fast as possible.
 * All heavy operations must take place in separate
 * threads. A JPEGWorker and a GTWorker are being used here.
 */
void DualFrameReceiver::framesReceived(const CameraFrame *_frameEye, const CameraFrame *_frameScene) {

	// copy the eye frame
	CameraFrameExtended *frameEye = new CameraFrameExtended(*_frameEye);


	pthread_mutex_lock(&mutex_receive);

    // increment the frame pair counter
    ++n_received_pairs;

    frameEye->id = n_received_pairs - 1UL;


    // first copy the videos to the saver
    video_writer->addFrames(_frameEye, _frameScene);


    /*
     * Depending upon the user's request, either both frames will be
     * placed to their designated worker queues for further processing
     * or just the eye frame will be placed to it's corresponding
     * worker's queue.
     */
    if(isGUIActive()) {

        /*
         * See if there is space in both workers' queues. If either
         * one is full, we will not add either frame to anywhere
         */
        const bool b_is_space = workers[0]->isSpace() && workers[1]->isSpace();


        /*
         * Only add the frames to the workers if both workers' queues allow
         * for that.
         */
        if(b_is_space) {

            // copy the eye frame
            CameraFrameExtended *frameScene = new CameraFrameExtended(*_frameScene);
            frameScene->id = n_received_pairs - 1UL;

            // add to workers
            workers[0]->add(frameEye);
            workers[1]->add(frameScene);

        }
        else {

            printf("DualFrameReceiver::framesReceived(): At least one of the worker's queue is full, skipping\n");
            delete frameEye;

        }

    }

    else { // GUI not active

        // see if the gaze tracker's worker queue has space
        const bool bIsSpace = workers[0]->isSpace();

        if(bIsSpace) {

            // add to worker
            workers[0]->add(frameEye);

        }
        else { // no space

            printf("DualFrameReceiver::framesReceived(): No space int the GTWorker's queue, skipping\n");

            delete frameEye;

        }

    }

	pthread_mutex_unlock(&mutex_receive);

}


// called by workers
bool DualFrameReceiver::frameProcessed(CameraFrame *_frame, void *user_data) {

	/*
	 * This function places the processed frames into the GUI queue,
	 * if the GUI is not active, then the frame must be deallocated
	 */
	if(!isGUIActive()) {

		delete _frame;

		// tell that we own this frame
		return true;

	}


	pthread_mutex_lock(&mutex_oput);

		const int id = *((int *)user_data);

		OutputData *oput = NULL;

		CameraFrameExtended *frame_extended = (CameraFrameExtended *)_frame;


		/* See if there is a pair waiting for this frame */
		std::list<OutputData *>::iterator it = list_oput.begin();
		while(it != list_oput.end()) {

			OutputData *cur_oput = *it;
			CameraFrameExtended **cur_frames = cur_oput->frames;

            // NULL means open slot
			if(cur_frames[id] == NULL) {

				cur_frames[id] = frame_extended;

				oput = cur_oput;

				break;

			}

			++it;

		}


		/* oput == NULL, when the frame has no pair waiting for it */
		if(oput == NULL) {

			// allocate memory for a new frame pair
			oput								= new OutputData();
			CameraFrameExtended **frame_pair	= oput->frames;
			frame_pair[id]						= frame_extended;

			/*
			 * If there is no room yet, wait. This takes place when
			 * one worker feeds frames faster than the other.
			 */

			if(list_oput.size() == MAX_BUFFER_SIZE) {

				printf("DualFrameReceiver::frameProcessed(): GUI queue full, removing the oldest...\n");

				std::list<OutputData *>::iterator itOldest = list_oput.begin();
				(*itOldest)->releaseFrames();
				delete *itOldest;

				list_oput.erase(itOldest);

			}

			list_oput.push_back(oput);

		}


		// if this function is being called by the GTWorker
		if(id == 0 && oput != NULL) {

			// swap the ownership of the track results from the frame to the output data
			oput->res = oput->frames[0]->res; // get the pointer from the frame to the output data
			oput->frames[0]->res = NULL;;     // set the frame's pointer to the results to NULL

			// write the results
			saveResults(oput);

		}


	pthread_mutex_unlock(&mutex_oput);


	// tell that we own this frame
	return true;

}


void DualFrameReceiver::saveResults(OutputData *oput) {

	std::vector<char> buff;
	BinaryResultParser::resDataToBuffer(*(oput->res), buff);

	if(buff.size() < BinaryResultParser::MIN_BYTES) {
		printf("DualFrameReceiver::saveResults(): %d\n", (int)buff.size());
	}

	// insert into file
	video_writer->addResults(buff);

}


OutputData *DualFrameReceiver::getData() {

	pthread_mutex_lock(&mutex_oput);

		OutputData *res = NULL;

		// get the oldest frame pair
		std::list<OutputData *>::iterator it = list_oput.begin();

		// see if the pair exists
		if(it != list_oput.end()) {

			OutputData *oput_data = *it;
			CameraFrameExtended **frames = oput_data->frames;

			// check if both images are present
			if(frames[0] != NULL && frames[1] != NULL) {

				res = oput_data;

				list_oput.erase(it);

				// frameProcessed() might be waiting by 2 workers, so broadcast.
				pthread_cond_broadcast(&cond_oput);

			}

		}

	pthread_mutex_unlock(&mutex_oput);

	return res;

}


OutputData *DualFrameReceiver::getNewestData() {

	OutputData *ret = NULL;

	pthread_mutex_lock(&mutex_oput);

		std::list<OutputData *>::reverse_iterator rit = list_oput.rbegin();

		while(rit != list_oput.rend()) {

			OutputData *curdata = *rit;
			CameraFrame *f1 = curdata->frames[0];
			CameraFrame *f2 = curdata->frames[1];


			if(f1 != NULL && f2 != NULL) {

				// how many to remove from the list
				int cdelete = 1;

				// assign the return value
				ret = curdata;

				++rit;

				while(rit != list_oput.rend()) {

					curdata = *rit;

					curdata->releaseFrames();
					delete curdata;

					++rit;

					++cdelete;
				}


				// delete the pointers from the list
				std::list<OutputData *>::iterator start	= list_oput.begin();
				std::list<OutputData *>::iterator end	= start;
				std::advance(end, cdelete);

				list_oput.erase(start, end);

				// frameProcessed() might be waiting by 2 workers, so broadcast.
				// actually not anymore... but there is no harm in broadcasting
				pthread_cond_broadcast(&cond_oput);

				break;

			}

			++rit;

		}

	pthread_mutex_unlock(&mutex_oput);

	return ret;

}


void DualFrameReceiver::collectForGUI(bool bCollect) {

	pthread_mutex_lock(&mutex_oput);

		bGUIActive = bCollect;

		if(!bCollect) {
	
			flushGUIQueue();

		}

	pthread_mutex_unlock(&mutex_oput);

}


bool DualFrameReceiver::isGUIActive() {

	pthread_mutex_lock(&mutex_oput);

		bool ret = bGUIActive;

	pthread_mutex_unlock(&mutex_oput);

	return ret;

}


/*
 * The mutex _must_ be locked and released manually, i.e. those
 * operations will _not_ be performed in this function
 */
void DualFrameReceiver::flushGUIQueue() {

	std::list<OutputData *>::iterator it = list_oput.begin();
	while(it != list_oput.end()) {

		OutputData *cur_oput = *it;
		cur_oput->releaseFrames();
		delete cur_oput;

		++it;

	}
	list_oput.clear();

}


void DualFrameReceiver::getWorkerBufferStates(size_t states[2]) {
	states[0] = workers[0]->getBufferState();
	states[1] = workers[1]->getBufferState();
}


int DualFrameReceiver::getMaxBufferSize() {
	return MAX_BUFFER_SIZE;
}


int DualFrameReceiver::getVideoBufferState() {

	pthread_mutex_lock(&mutex_oput);
		int sz = (int)list_oput.size();
	pthread_mutex_unlock(&mutex_oput);

	return sz;

}


int DualFrameReceiver::getSaveBufferState() {

	return video_writer->getBufferState();

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
	 *********************************************************************/
	CalibDataReader calibReader;
	if(!calibReader.create(eyeCamCalibFile)) {

		printf("DualFrameReceiver::configureEyeCamData(): Could not load \"%s\"\n",
				eyeCamCalibFile.c_str());

		return false;

	}


	/**********************************************************************
	 * Read the camera calibration container and assign the values to
	 * the camera
	 *********************************************************************/
	{

		calib::CameraCalibContainer camContainer;
		if(!calibReader.readCameraContainer(camContainer)) {

			printf("DualFrameReceiver::configureEyeCamData(): Could not read the camera container\n");

			return false;

		}


		/**********************************************************************
		 * Create the eye camera
		 *********************************************************************/

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


    m_vecLEDPositions.resize(nLEDs);


    // Go through each of the LEDs
    for(int i = 0; i < nLEDs; ++i) {

        // get the current container
        const calib::LEDCalibContainer &curLEDContainer = LEDContainers[i];

        double x = curLEDContainer.LED_pos[0];
        double y = curLEDContainer.LED_pos[1];
        double z = curLEDContainer.LED_pos[2];

        // .. store it
        m_vecLEDPositions[i] = cv::Point3d(x, y, z);

    }


	/**********************************************************************
	 * Create and initialise the tracker
	 *********************************************************************/
	tracker = new gt::GazeTracker();
	tracker->init(*camEye, m_vecLEDPositions);

	return true;

}


bool DualFrameReceiver::configureSceneCamData(const std::string &sceneCamCalibFile) {

	/**********************************************************************
	 * Open the settings file and read the settings of the calib reader
	 *********************************************************************/
	CalibDataReader calibReader;
	if(!calibReader.create(sceneCamCalibFile)) {

		printf("DualFrameReceiver::configureSceneCamData(): Could not load \"%s\"\n",
				sceneCamCalibFile.c_str());

		return false;

	}


	/**********************************************************************
	 * Read the camera calibration container and assign the values to
	 * the camera
	 *********************************************************************/
	calib::CameraCalibContainer camContainer;
	if(!calibReader.readCameraContainer(camContainer)) {

		printf("DualFrameReceiver::configureSceneCamData(): Could not read the camera container\n");

		return false;

	}

	/**********************************************************************
	 * Create and initialise the camera
	 *********************************************************************/
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

