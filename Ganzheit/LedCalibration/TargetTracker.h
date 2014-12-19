#ifndef TARGETTRACKER_H
#define TARGETTRACKER_H

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>


class TargetTracker {
	public:
		virtual bool track(cv::Mat &_img_gray, std::vector<cv::Point2f> &_polygon_rect, unsigned char _th, double &_target_x, double &_target_y, cv::Rect &_drawable_ellipse) = 0;
};


#endif

