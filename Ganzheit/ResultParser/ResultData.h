#ifndef RESULT_DATA_H
#define RESULT_DATA_H


#include <opencv2/imgproc/imgproc.hpp>
#include <list>


/*
 * Container for the data gaze tracking results.
 */
class ResultData {

	public:

		ResultData();

		bool bTrackSuccessfull;								// was the tracking successfull
		unsigned long id;									// id
		time_t timestamp;									// time stamp, seconds from Jan 1, 1970
		long trackDurMicros;								// track duration in microseconds
		cv::RotatedRect ellipsePupil;						// estimated ellipse of the pupil
		cv::Point3d corneaCentre;							// cornea centre
		cv::Point3d pupilCentre;							// pupil centre
		cv::Point2d scenePoint;								// the mapped scene image point
		std::vector<cv::Point2d> listGlints;				// glint centres
		std::vector<std::vector<cv::Point> > listContours;	// contours
		cv::Point gazeVecStartPoint2D;						// the gaze vector start point in the image, 2D
		cv::Point gazeVecEndPoint2D;						// the gaze vector end point in the image, 2D

		void clear();

};



#endif

