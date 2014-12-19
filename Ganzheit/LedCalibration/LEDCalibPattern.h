#ifndef LEDCALIBPATTERN_H
#define LEDCALIBPATTERN_H


#include "imgproc.h"


namespace calib {

    namespace LEDCalibPattern {


        bool findMarkers(const cv::Mat &img_gray,
                         unsigned char th,
                         cv::vector<cv::Point2f> &image_points);

        void computeNormal(const Eigen::Matrix4d &transformation,
                           Eigen::Vector3d &normal);


        /*
         * Returns the 16 points, with the given circle spacing.
         * The coordinates are in the OpenCV coordinate system.
         */
        void getLEDObjectPoints(double circleSpacing, std::vector<cv::Point3f> &objectPoints);

        /*
         * Returns the 16 points, with circle spacing set to 1.
         * Real object points can be retrieved by multiplying
         * the coordinates by the circle spacing. The coordinates are in the
         * OpenCV coordinate system.
         *
         * NOTE: This has to be implemented here, for now at least, so that the use of
         * LEDCalibSample is independent of LEDCalibPattern.o. Also this has to be inlined
         * to avoid multiple definitions.
         */
        inline void getNormalisedLEDObjectPoints(std::vector<cv::Point3f> &norm_object_points) {

            norm_object_points.resize(16);

            // 0..4 and 8..12
            for(int i = 0; i < 5; ++i) {
                norm_object_points[i]	= cv::Point3f(-2.0, -2.0 + i, 0.0);
                norm_object_points[8+i]	= cv::Point3f( 2.0,  2.0 - i, 0.0);
            }

            // 5..7 and 13..15
            for(int i = 0; i < 3; ++i) {
                norm_object_points[5+i]		= cv::Point3f(-1.0 + i,  2.0, 0.0);
                norm_object_points[13+i]	= cv::Point3f( 1.0 - i, -2.0, 0.0);
            }

        }



        /*
         *	The object points are in the OpenCV coordinate system
         */
        void makeTransformationMatrix(const cv::Mat &cv_intr,
                                      const cv::Mat &cv_dist,
                                      const std::vector<cv::Point2f> &image_points,
                                      const std::vector<cv::Point3f> &object_points,
                                      Eigen::Matrix4d &transformation);

        std::string getLastError();


    }

}


#endif

