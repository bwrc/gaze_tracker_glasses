#ifndef CAMERACALIBRATOR_H
#define CAMERACALIBRATOR_H


#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>
#include "Camera.h"
#include <list>
#include "CameraCalibSample.h"

namespace calib {


    namespace CameraCalibrator {

        enum PatternDim {

            COLS = 4,
            ROWS = 11,
            BOARD_N = COLS * ROWS

        };


		bool findCircles(const cv::Mat &img_gray, CameraCalibSample &sample);

        bool calibrateCamera(CameraCalibContainer &container);

        cv::Size getBoardSize();

		/* Get the object points */
		void getObjectPoints(std::vector<cv::Point3f> &_object_points);



    }


}	// end of namespace calib


#endif

