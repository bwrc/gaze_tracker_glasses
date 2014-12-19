#ifndef LEDTRACKER_H
#define LEDTRACKER_H


#include "TargetTracker.h"


class LEDTracker : public TargetTracker {
	public:
		bool track(cv::Mat &_img_gray, std::vector<cv::Point2f> &_polygon_rect, unsigned char _th, double &_target_x, double &_target_y, cv::Rect &_drawable_ellipse);


};


#endif

