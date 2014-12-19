#include "GTWorker.h"
#include "DualFrameReceiver.h"
#include "ResultData.h"


extern pthread_mutex_t mutex_tracker;


GTWorker::GTWorker() : JPEGWorker() {}



void GTWorker::setTracker(gt::GazeTracker *_tracker, SceneMapper *_mapper) {

	tracker = _tracker;
	mapper = _mapper;

}


CameraFrame *GTWorker::process(CameraFrame *img_compr) {

	/* dcompress the frame */
	CameraFrame *frame;

    if(img_compr->format == FORMAT_MJPG) {

        frame = JPEGWorker::process(img_compr);

    }
    else {

        frame = new CameraFrame(img_compr->w,
                                img_compr->h,
                                img_compr->bpp,
                                img_compr->data,
                                img_compr->sz,
                                img_compr->format,
                                true,	 // copy data
                                false);	 // do not become parent, i.e. do not destroy data in destructor

    }


	/* Analyse the frame */

	// make a cv::Mat header of the CameraFrame
	cv::Mat ocvFrame24(
		frame->h,				// rows
		frame->w,				// cols
		CV_8UC3,				// type of data
		frame->data,			// data
		frame->w * frame->bpp	// bytes per row
	);

	// convert the 24-bit image to gray
	int conversionType = frame->format == FORMAT_RGB ? CV_RGB2GRAY : CV_BGR2GRAY;
	cv::Mat ocvFrameGray;
	cv::cvtColor(ocvFrame24, ocvFrameGray, conversionType);

    // flip around y-axis
    cv::flip(ocvFrameGray, ocvFrameGray, 1);

	// track the grayscale image
	pthread_mutex_lock(&mutex_tracker);

		/***************************************************************
		 * Track gaze
		 **************************************************************/
		const bool trackSuccess = tracker->track(ocvFrameGray);


		/***************************************************************
		 * Map the gaze vector to the scene
		 **************************************************************/
		const double *c_pupil	= tracker->getCentrePupil();
		const double *c_cornea	= tracker->getCentreCornea();
		Eigen::Vector3d eigCornea(c_cornea[0], c_cornea[1], c_cornea[2]);
		Eigen::Vector3d eigPupil(c_pupil[0], c_pupil[1], c_pupil[2]);

		// Map the gaze to the scene
		cv::Point2d scenePoint;
		mapper->getPosition(eigCornea, eigPupil, scenePoint);


		/***************************************************************
		 * Compute gaze vector in 2D
		 ***************************************************************/
		const Camera &cam = tracker->getCamera();

		// convert to OpenCV container
		cv::Point3d p_cornea(c_cornea[0], c_cornea[1], c_cornea[2]);

		// get the corresponding pixel
		double u1, v1;
		cam.worldToPix(p_cornea, &u1, &v1);

		// convert to OpenCV container
		cv::Point3d p_pupil(c_pupil[0], c_pupil[1], c_pupil[2]);


		cv::Point3d pupil_to_cornea(
				p_pupil.x - p_cornea.x,
				p_pupil.y - p_cornea.y,
				p_pupil.z - p_cornea.z
		);

		pupil_to_cornea *= 3.0;
		double u2, v2;
		cam.worldToPix(pupil_to_cornea + p_cornea, &u2, &v2);


		/***************************************************************
		 * Create and store results
		 **************************************************************/
		unsigned long id = ((CameraFrameExtended *)(img_compr))->id;

		gt::PupilTracker *pupilTracker = tracker->getPupilTracker();

		// create the results object
		ResultData *tr = new ResultData();

		// copy the results
		tr->id					= id;
		tr->timestamp			= time(NULL);
		tr->listContours		= pupilTracker->getClusters();
		tr->ellipsePupil		= *pupilTracker->getEllipsePupil();
		tr->listGlints			= pupilTracker->getCornealReflections();
		tr->trackDurMicros		= tracker->getTrackDurationMicros();
		tr->scenePoint			= scenePoint;
		tr->bTrackSuccessfull	= trackSuccess;
		tr->pupilCentre			= cv::Point3d(c_pupil[0], c_pupil[1], c_pupil[2]);
		tr->corneaCentre		= cv::Point3d(c_cornea[0], c_cornea[1], c_cornea[2]);
		tr->gazeVecStartPoint2D	= cv::Point(u1, v1);
		tr->gazeVecEndPoint2D	= cv::Point(u2, v2);

	pthread_mutex_unlock(&mutex_tracker);


	CameraFrameExtended *frame_extended = new CameraFrameExtended(
												id,
												tr,
												frame->w,
												frame->h,
												frame->bpp,
												frame->data,
												frame->sz,
												frame->format,
												false,			// do not copy
												true);			// become parent

	// keep data, delete all other things
	frame->b_own_data = false;
	delete frame;

	return frame_extended;

}

