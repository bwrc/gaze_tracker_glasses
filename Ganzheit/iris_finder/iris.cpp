/*
 *
 * cvEyeTracker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * cvEyeTracker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cvEyeTracker; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * cvEyeTracker - Version 1.2.5
 * Part of the openEyes ToolKit -- http://hcvl.hci.iastate.edu/openEyes
 * Release Date:
 * Authors : Dongheng Li <dhli@iastate.edu>
 *           Derrick Parkhurst <derrick.parkhurst@hcvl.hci.iastate.edu>
 *           Jason Babcock <babcock@nyu.edu>
 *           David Winfield <dwinfiel@iastate.edu>
 * Copyright (c) 2004-2006
 * All Rights Reserved.
 *
 */


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <cstring>
#include <iostream>

#include "iris.h"
#include "trackerSettings.h"
#include "ellipse.h"

static const double PI        = 3.141592653589;
static const int EDGE_THR_MIN = 1;
static const int EDGE_THR_MAX = 10;

static const size_t nRays         = 360;
static const size_t nMinFoundRays = 20;

namespace iris {


    enum ERR {
        UNRECOVERABLE_ERROR,
        OK
    };

    static double calculateStatistics(cv::Point2f &center, const std::vector<cv::Point2f> &);

    static bool testThresholds(const cv::Mat &img_gray,
                               const cv::Rect &roi,
                               int &threshold,
                               std::vector<cv::Point2f> &vecEdges,
                               double &pointVariance,
                               const cv::RotatedRect &ellipse);

    static bool starburst_pupil_contour_detection(const cv::Mat &img_gray,
                                                  const cv::Rect &roi,
                                                  int edge_thresh,
                                                  const cv::RotatedRect &ellipse,
                                                  std::vector<cv::Point2f> &vecEdges,
                                                  int N);

    static void locate_edge_points(const cv::Mat &img_gray,
                                   const cv::Rect &roi,
                                   const cv::RotatedRect &ellipse,
                                   int dis,
                                   double angle_step,
                                   double angle_spread,
                                   int edge_thresh,
                                   std::vector<cv::Point2f> & pointList);

    static void removeOutliers(cv::Point2f &mean, double var, std::vector<cv::Point2f> &vecEdges);



    bool burst(const cv::Mat &img_gray,
               const cv::Rect &roi,
               const cv::RotatedRect &ellipse,
               std::vector<cv::Point2f> &vecEdges) {

        // set a valid ROI
        cv::Rect validRoi;

        if(roi.width == 0 || roi.height == 0) {
            validRoi.x      = 0;
            validRoi.y      = 0;
            validRoi.width  = img_gray.cols;
            validRoi.height = img_gray.rows;
        }
        else {
            validRoi = roi;
        }


        double dVarOfOrigImg;
        int threshold;
        if(!testThresholds(img_gray,
                           validRoi,
                           threshold,
                           vecEdges,
                           dVarOfOrigImg,
                           ellipse)) {

            return UNRECOVERABLE_ERROR;
        }


        if(!starburst_pupil_contour_detection(img_gray,
                                              validRoi,
                                              threshold,
                                              ellipse,
                                              vecEdges,
                                              nRays)) {

            return UNRECOVERABLE_ERROR;
        }

        cv::Point2f center;
        double var = calculateStatistics(center, vecEdges);

        if(var < 0 || vecEdges.size() == 0) {

            return UNRECOVERABLE_ERROR;
        }

        //        removeOutliers(center, var, vecEdges);

        var = calculateStatistics(center, vecEdges);
        if(var < 0 || vecEdges.size() == 0) {

            return UNRECOVERABLE_ERROR;
        }

        double sd_difference = std::abs(std::sqrt(var) - std::sqrt(dVarOfOrigImg));

        if(var < 0 || vecEdges.size() == 0 || 
           (sd_difference > std::sqrt(dVarOfOrigImg) * 
            trackerSettings.STARBURST_RELATIVE_MAX_POINT_VARIANCE)) {

            return UNRECOVERABLE_ERROR;
        }


        return OK;
    }


    bool testThresholds(const cv::Mat &img_gray,
                        const cv::Rect &roi,
                        int &thresholdOut,
                        std::vector<cv::Point2f> &vecEdges,
                        double &pointVarianceOut,
                        const cv::RotatedRect &ellipse) {

        // best values will be stored in these
        double      dBestVar       = -1;
        int         nBestThreshold =  1;


        // Try different thresholds
        for(int nCurTh = EDGE_THR_MIN; nCurTh <= EDGE_THR_MAX; ++nCurTh) {

            if(!starburst_pupil_contour_detection(img_gray,
                                                  roi,
                                                  nCurTh,
                                                  ellipse,
                                                  vecEdges,
                                                  nRays)) {

                continue;

            }

            cv::Point2f center;
            double var = calculateStatistics(center, vecEdges);

            /*
             * Store the best values if the var is smaller than the current
             * smallest value.
             */
            if((var >= 0) && (dBestVar < 0 ||
                              std::sqrt(var) < std::sqrt(dBestVar))) {

                dBestVar = var;
                nBestThreshold = nCurTh;

            }

        }

        // non-zero if at least one threshold value was evaluated
        if(dBestVar < 0) {
            return false;
        }

        // write to output
        thresholdOut     = nBestThreshold;
        pointVarianceOut = dBestVar;

        return true;

    }


    //------------ Burst pupil edge detection -----------//

    // Input
    // pupile_image: input image
    // width, height: size of the input image
    // cx,cy: central start point of the feature detection process
    // pupil_edge_threshold: best guess for the pupil contour threshold 
    // N: number of rays 
    // minimum_candidate_features: must return this many features or error
    bool starburst_pupil_contour_detection(const cv::Mat &img_gray,
                                           const cv::Rect &roi,
                                           int edge_thresh,
                                           const cv::RotatedRect &ellipse,
                                           std::vector<cv::Point2f> &vecEdges,
                                           int N) {

        const int dis = 7;

        // get an empty list for this iteration
        vecEdges.clear();

        /*************************************************************
         * Get the edges
         ************************************************************/
        locate_edge_points(img_gray, roi, ellipse,
                           dis, (2.0 * PI) / (double)(N-1), 2.0 * PI,
                           edge_thresh, vecEdges);

        // Is there enough seed points?
        if(vecEdges.size() < nMinFoundRays) {
            return false;
        }

        return true;

    }


    // static method
    void locate_edge_points(const cv::Mat &img_gray,
                            const cv::Rect &roi,
                            const cv::RotatedRect &ellipse,
                            int dis, double angle_step, double angle_spread,
                            int edge_thresh, std::vector<cv::Point2f> &pointList) {

        // inclusive bounds
        const int xEnd = roi.x + roi.width - 1;
        const int yEnd = roi.y + roi.height - 1;

        const double dAngStart = -angle_spread * 0.5 + 0.0001;
        const double dAngEnd   =  angle_spread * 0.5;

        for(double angle = dAngStart; angle <= dAngEnd; angle += angle_step) {

            /*
             * In order to get an ellipse perimeter point at an angle ang,
             * defined in the image coordinate system, we must compute what
             * this angle corresponds to in the ellipses coordinate system,
             * as getEllipsePoint*() requires the angle to be defined that way.
             */
            const double dAngCompensateRad = angle - ellipse.angle * 3.141592653589 / 180.0;
            cv::Point2d p = ellipse::getEllipsePoint(ellipse, dAngCompensateRad);

            const double dis_cos = dis * cos(angle);
            const double dis_sin = dis * sin(angle);

            p.x += dis_cos;
            p.y += dis_sin;

            if(p.x < roi.x || p.x > xEnd || p.y < roi.y || p.y > yEnd) {
                continue;
            }

            int pixel_value1 = img_gray.data[(int)(p.y)*img_gray.step+(int)(p.x)];
            while(1) {
                p.x += dis_cos;
                p.y += dis_sin;
                if (p.x < roi.x || p.x > xEnd || p.y < roi.y || p.y > yEnd)
                    break;

                const int pixel_value2 = img_gray.data[(int)(p.y)*img_gray.step+(int)(p.x)];

                if (pixel_value2 - pixel_value1 > edge_thresh) {
                    pointList.push_back(cv::Point2f(p.x - dis_cos * 0.5, p.y - dis_sin * 0.5));
                    break;
                }

                pixel_value1 = pixel_value2;

            }

        }

    }


    void removeOutliers(cv::Point2f &mean, double var, std::vector<cv::Point2f> &vecEdges) {

        const double stdDev = std::sqrt(var);

        std::vector<cv::Point2f>::iterator edge;
        for(edge = vecEdges.begin(); edge != vecEdges.end();) {

            double diff_x = edge->x - mean.x;
            double diff_y = edge->y - mean.y;

            if(std::sqrt(diff_x * diff_x + diff_y * diff_y) > (stdDev * 
                                                               trackerSettings.STARBURST_OUTLIER_DISTANCE)) {

                // Take the element to remove
                std::vector<cv::Point2f>::iterator remove_pos = edge;

                // Is this the first element?
                if(remove_pos != vecEdges.begin()) {
                    // No.. just remove it
                    edge--;
                    vecEdges.erase(remove_pos);
                } else {
                    // We're removing the first element... remove and reset iterator.
                    vecEdges.erase(remove_pos);
                    edge = vecEdges.begin();
                    continue;
                }
            }

            // Next element (NOTE! This must not be run if we are dealing with the first element!)
            edge++;
        }

    }


    double calculateStatistics(cv::Point2f &center, const std::vector<cv::Point2f> &vecEdges)
    {
        double square_x = 0, square_y = 0;

        center = cv::Point2f(0.0, 0.0);

        if(vecEdges.size() != 0) {

            std::vector<cv::Point2f>::const_iterator itEdge;
            for(itEdge = vecEdges.begin();
                itEdge != vecEdges.end();
                itEdge++) {

                center   += *itEdge;
                square_x += itEdge->x * itEdge->x;
                square_y += itEdge->y * itEdge->y;

            }

            const size_t nSz = vecEdges.size();

            center.x /= nSz;
            center.y /= nSz;

            square_x /= nSz;
            square_y /= nSz;

            double var_x = (center.x * center.x - square_x);
            double var_y = (center.y * center.y - square_y);

            return std::sqrt(var_x * var_x + var_y * var_y);

        }

        return -1;
    }


} // end of "namespace Burst"

