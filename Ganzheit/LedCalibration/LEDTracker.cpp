#include "LEDTracker.h"



// check if the coord is within the rect defined by corners
bool isInRect(int x, int y, std::vector<cv::Point2f> &_polygon_rect) {

	if(_polygon_rect.size() != 4)  {return false;}

	if(pointPolygonTest(_polygon_rect, cv::Point2f(x, y), false) < 0)
		return false;
	return true;
}


bool LEDTracker::track(cv::Mat &_img_gray,
                       std::vector<cv::Point2f> &_polygon_rect,
                       unsigned char _th,
                       double &_target_x,
                       double &_target_y,
                       cv::Rect &_drawable_ellipse) {

	int off			= 10;
	int row_start	= std::min(_polygon_rect[0].y, _polygon_rect[3].y) + off;
	int row_end		= std::max(_polygon_rect[1].y, _polygon_rect[2].y) - off;
	int col_start	= std::min(_polygon_rect[0].x, _polygon_rect[1].x) + off;
	int col_end		= std::max(_polygon_rect[2].x, _polygon_rect[3].x) - off;

	const unsigned char * const data = _img_gray.data;

	// Sanity check
	if(col_start < 0 ||
       row_start < 0 ||
       (col_end - col_start) < 0 ||
       (row_end - row_start) < 0) {

		return false;
	}


	/**********************************************************************
	 * Find the brightest pixel inside the rectangle.
	 *********************************************************************/

    int step = _img_gray.step;
	unsigned char *bright = (unsigned char *)&data[row_start * step + col_start];
	for(int row = row_start; row <= row_end; ++row) {
		for(int col = col_start; col <= col_end; ++col) {
			if((data[row*step + col] > *bright) && isInRect(col, row, _polygon_rect)) {
				bright = (unsigned char *)&data[row*step + col];
			}
		}
	}

	// get the coords of the brightest pixel
	int x = (bright - data) % step;
	int y = (bright - data) / step;


	/**********************************************************************
	 * Threshold the image
	 *********************************************************************/

    cv::Rect roi(col_start,
                 row_start,
                 col_end - col_start + 1,
                 row_end - row_start + 1);

	cv::Mat imgBinary;
	cv::threshold(cv::Mat(_img_gray, roi),
                  imgBinary,
                  _th,
                  255,
                  cv::THRESH_BINARY);


	/**********************************************************************
	 * Find the contours
	 *********************************************************************/

	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(imgBinary,
                     contours,
                     CV_RETR_LIST,
                     CV_CHAIN_APPROX_SIMPLE,
                     cv::Point(col_start, row_start));


	/**********************************************************************
	 * Find the best circle inside the rectangle
	 *********************************************************************/

    size_t contour_idx;

	// test each contour
	for(contour_idx = 0; contour_idx < contours.size(); contour_idx++) {

		// Is the contour large enough?
		if(contours[contour_idx].size() < 5)
			continue;

		// Does the shape include the brightest point?
		if(pointPolygonTest(contours[contour_idx], cv::Point2f(x, y), true) < -25)
			continue;

		/*
         * Make sure that all the pixels of the contour are inside
		 * the rectangle
         */

        bool valid = true;
        for(size_t j = 0; j < contours[contour_idx].size(); j++) {
            if(!isInRect(contours[contour_idx][j].x, contours[contour_idx][j].y, _polygon_rect)) {
                valid = false;
                break;
            }

        }

        if(!valid) {
            continue;
        }

        break;

    }

	/**********************************************************************
	 * Copy the results (..if any)
	 *********************************************************************/

	if(contour_idx < contours.size()) {

		// Calculate the center of the shape
		cv::RotatedRect ellipse = cv::fitEllipse(contours[contour_idx]);
		_drawable_ellipse = ellipse.boundingRect();


		// Get the minimum and maximum values from the contour area. These are used for data normalization
		int min = 255;
		int max = 0;
		for(y = _drawable_ellipse.y; y < _drawable_ellipse.y + _drawable_ellipse.height; y++) {
			for(x = _drawable_ellipse.x; x < _drawable_ellipse.x + _drawable_ellipse.width; x++) {

				if(x < 0 || x >= _img_gray.cols || y < 0 || y >= _img_gray.rows) {
					continue;
				}

				if(pointPolygonTest(contours[contour_idx], cv::Point2f(x, y), false) >= 0) {

                    const unsigned char val = data[x + y * step];
                    if(val < min) {
						min = val;
                    }

					if(val > max) {
						max = val;
                    }

				}
			}
		}

		// (Scale also the maximum value)
		max = max - min;
		if(!max) {
			return false;
        }

		double x_weighted = 0.0;
		double y_weighted = 0.0;
		double data_sum   = 0.0;

		for(y = _drawable_ellipse.y; y < _drawable_ellipse.y + _drawable_ellipse.height; y++) {
			for(x = _drawable_ellipse.x; x < _drawable_ellipse.x + _drawable_ellipse.width; x++) {

				if(x < 0 || x >= _img_gray.cols || y < 0 || y >= _img_gray.rows) {
					continue;
				}

				if(pointPolygonTest(contours[contour_idx], cv::Point2f(x, y), false) >= 0) {

					// Get the normalized value for the data
					double scaled = (double)(data[x + y * _img_gray.step] - min) / (double)max;

					x_weighted += scaled * x;
					y_weighted += scaled * y;
					data_sum += scaled;

				} 
			}
		}

		// _target_x = _drawable_ellipse.x + _drawable_ellipse.width / 2;
		// _target_y = _drawable_ellipse.y + _drawable_ellipse.height / 2;


        _target_x = x_weighted / (double)data_sum;
        _target_y = y_weighted / (double)data_sum;


		return true;

	}

	return false;

}

