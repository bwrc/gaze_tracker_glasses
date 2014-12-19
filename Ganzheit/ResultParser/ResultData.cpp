#include "ResultData.h"


ResultData::ResultData() {

	clear();

}

void ResultData::clear() {

	bTrackSuccessfull = false;
	id = 0;
	timestamp = 0;
	trackDurMicros = 0;
	ellipsePupil = cv::RotatedRect();
	corneaCentre = cv::Point3d();
	pupilCentre = cv::Point3d();
	scenePoint = cv::Point2d();
	listGlints.clear();
	listContours.clear();
	gazeVecStartPoint2D = cv::Point();
	gazeVecEndPoint2D = cv::Point();

}

