#include "CameraCalibrator.h"
#include <string.h>
#include <opencv2/calib3d/calib3d.hpp>
#include <stdio.h>




/* minimum number of calib pics to gather */
static const int N_BOARDS = 20;



namespace calib {

    namespace CameraCalibrator {

        cv::Size getBoardSize() {return cv::Size(COLS, ROWS);}


        /*
         *	Find the circles of the given image. Note that this function just updates the circles but does not
         *	write the results anywhere. The user must call addCalibData() in order to store the
         *	results.
         */
        bool findCircles(const cv::Mat &img_gray,
                                           CameraCalibSample &sample) {

            // clear any old results
            sample.clear();

            cv::Size boardSz = cv::Size(COLS, ROWS);

            return findCirclesGrid(img_gray,
                                   boardSz,
                                   sample.image_points,
                                   cv::CALIB_CB_ASYMMETRIC_GRID);

        }


        void getObjectPoints(std::vector<cv::Point3f> &_object_points) {

            _object_points.clear();

            // for an asymmetric pattern
            for(int row = 0; row < ROWS; ++row) {

                for(int col = 0; col < COLS; ++col) {

                    _object_points.push_back(
                                      cv::Point3f(float((2*col + row % 2)*1.0f),  // x
                                                  float(row*1.0f),                // y	
                                                  0.0f));                         // z

                }

            }

        }


        /*
         *	http://dasl.mem.drexel.edu/~noahKuntz/openCVTut10.html
         *	Perform calibration and put the results to the intrinsic and distortion matrices
         */
        bool calibrateCamera(CameraCalibContainer &container) {

            // the image size must be valid
            if(container.imgSize == cv::Size(0, 0)) {

                return false;

            }

            // we need at least N_BOARDS calibration results
            if((int)container.getSamples().size() < N_BOARDS) {

                return false;

            }

            /****************************************************************
             * Image points and object points
             ****************************************************************/
            const std::vector<CameraCalibSample> &samples = container.getSamples();
            size_t sz = samples.size();
            std::vector<std::vector<cv::Point2f> > image_points(sz);
            std::vector<std::vector<cv::Point3f> > object_points(sz);

            // must create as many copies of object points as there are image point vectors
            for(size_t i = 0; i < sz; ++i) {

                image_points[i]		= samples[i].image_points;
                object_points[i]	= container.object_points;

            }


            /****************************************************************
             * Calibrate:
             *
             *   Finds intrinsic and extrinsic camera parameters from
             *   a few views of known calibration patterns.
             ****************************************************************/
            cv::Mat distortion_coeffs	= cv::Mat::zeros(5, 1, CV_64F);
            cv::Mat intrinsic_matrix	= cv::Mat::eye(3, 3, CV_64F);

            // initialize intrinsic matrix such that the two focal lengths have a ratio of 1.0
            intrinsic_matrix.at<double>(0, 0) = 1.0;
            intrinsic_matrix.at<double>(1, 1) = 1.0;

            std::vector<cv::Mat> rvecs, tvecs;

            double error = cv::calibrateCamera(object_points,
                                               image_points,
                                               container.imgSize,
                                               intrinsic_matrix,
                                               distortion_coeffs,
                                               rvecs,
                                               tvecs/*,
                                                      CV_CALIB_FIX_ASPECT_RATIO*/);


            container.reproj_err = error;

            // column-major
            container.intr[0] = intrinsic_matrix.at<double>(0, 0);
            container.intr[1] = intrinsic_matrix.at<double>(1, 0);
            container.intr[2] = intrinsic_matrix.at<double>(2, 0);
            container.intr[3] = intrinsic_matrix.at<double>(0, 1);
            container.intr[4] = intrinsic_matrix.at<double>(1, 1);
            container.intr[5] = intrinsic_matrix.at<double>(2, 1);
            container.intr[6] = intrinsic_matrix.at<double>(0, 2);
            container.intr[7] = intrinsic_matrix.at<double>(1, 2);
            container.intr[8] = intrinsic_matrix.at<double>(2, 2);

            container.dist[0] = distortion_coeffs.at<double>(0);
            container.dist[1] = distortion_coeffs.at<double>(1);
            container.dist[2] = distortion_coeffs.at<double>(2);
            container.dist[3] = distortion_coeffs.at<double>(3);
            container.dist[4] = distortion_coeffs.at<double>(4);

            return true;

        }


    }

}	// end of namespace calib

