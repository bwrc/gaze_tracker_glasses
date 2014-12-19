#ifndef RECT_PATTERN_H
#define RECT_PATTERN_H


#include "Pattern.h"



class RectPattern : public Pattern {
	public:
		RectPattern(double _width);


		bool findMarkers(const cv::Mat &img_gray);
		void computeNormal();
};


#endif

