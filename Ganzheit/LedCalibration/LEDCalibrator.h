#ifndef LEDCALIBRATOR_H
#define LEDCALIBRATOR_H


#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <algorithm>
#include <list>
#include "Camera.h"
#include "RectangleEstimator.h"
#include "LEDCalibPattern.h"
#include <stdio.h>
#include "LEDCalibSample.h"


namespace calib {


    /*****************************************************
     * Calibration related stuff
     *****************************************************/

    Eigen::Vector3d computeIntersectionPoint(const cv::Point2d &glint,
                                             const std::vector<cv::Point3f> objectPoints,
                                             const Eigen::Vector3d &eig_normal,
                                             const Eigen::Matrix4d &transformation,
                                             const Camera &cam);


    Eigen::Vector3d computeReflectionPoint(const Eigen::Vector3d &normal,
                                           const Eigen::Vector3d &S);


    bool extractFeatures(const cv::Mat &img_rgb,
                         LEDCalibSample &resSample,
                         unsigned char thRect,
                         unsigned char thCr);


    /* Perform the calibration */
    bool calibrateLED(const LEDCalibContainer &container, const Camera &cam);


}	// end of namespace calib


#endif

