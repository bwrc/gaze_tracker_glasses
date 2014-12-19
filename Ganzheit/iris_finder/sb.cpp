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

#include "sb.h"
#include "trackerSettings.h"

static const double PI = 3.141592653589;
static const int EDGE_THR_MIN = 1;
static const int EDGE_THR_MAX = 40;

namespace Burst {


    enum ERR {
        UNRECOVERABLE_ERROR,
        OK
    };

    static double calculateStatistics(cv::Point2f &center, const std::list<EdgePoint> &);

    static bool testThresholds(const cv::Mat &img_gray,
                               const cv::Rect &roi,
                               int &threshold,
                               std::list<EdgePoint> &listEdges,
                               double &pointVariance,
                               cv::Point2f &startPointInOut);

    static bool starburst_pupil_contour_detection(const cv::Mat &img_gray,
                                                  const cv::Rect &roi,
                                                  int edge_thresh,
                                                  const cv::Point2f &startPoint,
                                                  std::list<EdgePoint> &listEdges,
                                                  int N,
                                                  unsigned int minimum_cadidate_features);

    static void locate_edge_points(const cv::Mat &img_gray,
                                   const cv::Rect &roi,
                                   double cx,
                                   double cy,
                                   int dis,
                                   double angle_step,
                                   double angle_normal,
                                   double angle_spread,
                                   int edge_thresh,
                                   std::list<EdgePoint> & pointList);

    static void removeOutliers(cv::Point2f &mean, double var, std::list<EdgePoint> &listEdges);
    static cv::Point2f get_edge_mean(std::list<EdgePoint> & pointList);


    bool burst(const cv::Mat &img_gray,
               const cv::Rect &roi,
               cv::Point2f startPoint,
               std::list<EdgePoint> &listEdges) {

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

        // starting point for the burst
        if(startPoint.x < validRoi.x || validRoi.x > validRoi.x + validRoi.width - 1 ||
           startPoint.y < validRoi.y || startPoint.y > validRoi.y + validRoi.height - 1) {

            startPoint.x = validRoi.x + (validRoi.width  >> 1);
            startPoint.y = validRoi.y + (validRoi.height >> 1);

        }


        double dVarOfOrigImg;
        int threshold;
        if(!testThresholds(img_gray,
                           validRoi,
                           threshold,
                           listEdges,
                           dVarOfOrigImg,
                           startPoint)) {

            return UNRECOVERABLE_ERROR;
        }


        if(!starburst_pupil_contour_detection(img_gray,
                                              validRoi,
                                              threshold,
                                              startPoint,
                                              listEdges,
                                              trackerSettings.STARBURST_CIRCULAR_STEPS,
                                              trackerSettings.STARBURST_MIN_SEED_POINTS)) {

            return UNRECOVERABLE_ERROR;
        }

        cv::Point2f center;
        double var = calculateStatistics(center, listEdges);

        if(var < 0 || listEdges.size() == 0) {

            return UNRECOVERABLE_ERROR;
        }

        removeOutliers(center, var, listEdges);

        var = calculateStatistics(center, listEdges);
        if(var < 0 || listEdges.size() == 0) {

            return UNRECOVERABLE_ERROR;
        }

        double sd_difference = std::abs(std::sqrt(var) - std::sqrt(dVarOfOrigImg));

        if(var < 0 || listEdges.size() == 0 || 
           (sd_difference > std::sqrt(dVarOfOrigImg) * 
            trackerSettings.STARBURST_RELATIVE_MAX_POINT_VARIANCE)) {

            return UNRECOVERABLE_ERROR;
        }


        return OK;
    }


    bool testThresholds(const cv::Mat &img_gray,
                        const cv::Rect &roi,
                        int &thresholdOut,
                        std::list<EdgePoint> &listEdges,
                        double &pointVarianceOut,
                        cv::Point2f &startPointInOut) {

        double lowest_var = -1;

        // Test different threshold values
        int best_threshold = 1;
        int numOfPoints = -1;

        cv::Point2f best_point(0, 0);


        // Try different thresholds
        for(int pupil_edge_thres = EDGE_THR_MIN;
            pupil_edge_thres <= EDGE_THR_MAX;
            pupil_edge_thres++) {

            if(!starburst_pupil_contour_detection(img_gray,
                                                  roi,
                                                  pupil_edge_thres,
                                                  startPointInOut,
                                                  listEdges,
                                                  trackerSettings.STARBURST_CIRCULAR_STEPS,
                                                  trackerSettings.STARBURST_MIN_SEED_POINTS)) {

                continue;

            }

            cv::Point2f center;
            double var = calculateStatistics(center, listEdges);

            /* If there happens to be a good threshold
             * with a smaller amount of points, pick it
             * (avoid selecting "almost perfect"
             * threshold value) */

            if((var >= 0) && (lowest_var < 0 ||
                              std::sqrt(var) < std::sqrt(lowest_var) * 
                              trackerSettings.STARBURST_RELATIVE_MAX_THRESHOLD_VARIANCE)) {

                // if((numOfPoints > (int)listEdges.size()) || (numOfPoints == -1)) {

                numOfPoints = listEdges.size();
                lowest_var = var;
                best_threshold = pupil_edge_thres;
                best_point = startPointInOut;

                //}

            }


        }

        if(lowest_var < 0) {
            return false;
        }


        thresholdOut     = best_threshold;
        pointVarianceOut = lowest_var;
        startPointInOut  = best_point;

        printf("best th: %d\n", best_threshold);

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
                                           const cv::Point2f &startPoint,
                                           std::list<EdgePoint> &listEdges,
                                           int N,
                                           unsigned int minimum_cadidate_features) {

        const int dis = 7;

        // spread of the bouncing rays
        const double angle_spread = 0.5*PI;

        const double angle_step = angle_spread / (double)(N - 1);

        double cx = startPoint.x;
        double cy = startPoint.y;

        int loop_count = 0;

        while(loop_count <= trackerSettings.STARBURST_MAX_ITERATIONS) {

            // get an empty list for this iteration
            listEdges.clear();

            /*************************************************************
             * First phase of the algorithm: Find the seed points
             ************************************************************/

            std::list<EdgePoint> seed_points;
            Burst::locate_edge_points(img_gray, roi, cx, cy,
                                      dis, (2.0 * PI) / (double)(N-1), 0, 2.0 * PI,
                                      edge_thresh, seed_points);

            // Is there enough seed points?
            if(seed_points.size() < minimum_cadidate_features) {

                // Store the found points for debugging purposes and return.
                listEdges = seed_points;
                return false;
            }


            /*************************************************************
             * Second phase of the algorithm: Find the points from the
             * returning rays.
             ************************************************************/

            std::list<EdgePoint>::iterator itEdge;
            for(itEdge = seed_points.begin(); itEdge != seed_points.end(); itEdge++) {

                double angle_normal = atan2(cy - itEdge->point.y, cx - itEdge->point.x);

                double new_angle_step = angle_step * ((double)edge_thresh / (double)itEdge->intensityDifference);

                Burst::locate_edge_points(img_gray,
                                          roi,
                                          itEdge->point.x,
                                          itEdge->point.y,
                                          dis,
                                          new_angle_step,
                                          angle_normal,
                                          angle_spread,
                                          edge_thresh,
                                          listEdges);

            }

            /*************************************************************
             * Process the results
             ************************************************************/

            // Merge seed points with edge_points.
            listEdges.splice(listEdges.begin(), seed_points);

            ++loop_count;
            cv::Point2f edge_mean = get_edge_mean(listEdges);

            // Converge?
            if((fabs(edge_mean.x-cx) + fabs(edge_mean.y-cy)) < trackerSettings.STARBURST_REQUIRED_ACCURACY &&
               loop_count > trackerSettings.STARBURST_MIN_ITERATIONS) {

                return true;
            }

            cx = edge_mean.x;
            cy = edge_mean.y;

        }

        return false;
    }


    // static method
    void locate_edge_points(const cv::Mat &img_gray,
                            const cv::Rect &roi,
                            double cx, double cy,
                            int dis, double angle_step, double angle_normal, double angle_spread,
                            int edge_thresh, std::list<EdgePoint> &pointList) {

        // inclusive bounds
        const int xEnd = roi.x + roi.width - 1;
        const int yEnd = roi.y + roi.height - 1;

        double dAngStart = angle_normal - angle_spread * 0.5 + 0.0001;
        double dAngEnd   = angle_normal + angle_spread * 0.5;

        for(double angle = dAngStart; angle <= dAngEnd; angle += angle_step) {

            const double dis_cos = dis * cos(angle);
            const double dis_sin = dis * sin(angle);

            cv::Point2f p(cx + dis_cos,
                          cy + dis_sin);

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
                    pointList.push_back(EdgePoint(cv::Point2f(p.x - dis_cos * 0.5, p.y - dis_sin * 0.5),
                                                  pixel_value2 - pixel_value1));
                    break;
                }

                pixel_value1 = pixel_value2;

            }

        }

    }


    void removeOutliers(cv::Point2f &mean, double var, std::list<EdgePoint> &listEdges) {

        const double stdDev = std::sqrt(var);

        std::list <EdgePoint>::iterator edge;
        for(edge = listEdges.begin(); edge != listEdges.end();) {

            double diff_x = edge->point.x - mean.x;
            double diff_y = edge->point.y - mean.y;

            if(std::sqrt(diff_x * diff_x + diff_y * diff_y) > (stdDev * 
                                                               trackerSettings.STARBURST_OUTLIER_DISTANCE)) {

                // Take the element to remove
                std::list <EdgePoint>::iterator remove_pos = edge;

                // Is this the first element?
                if(remove_pos != listEdges.begin()) {
                    // No.. just remove it
                    edge--;
                    listEdges.erase(remove_pos);
                } else {
                    // We're removing the first element... remove and reset iterator.
                    listEdges.erase(remove_pos);
                    edge = listEdges.begin();
                    continue;
                }
            }

            // Next element (NOTE! This must not be run if we are dealing with the first element!)
            edge++;
        }
    }


    double calculateStatistics(cv::Point2f &center, const std::list<EdgePoint> &listEdges)
    {
        double square_x = 0, square_y = 0;

        center = cv::Point2f(0.0, 0.0);

        if(listEdges.size() != 0) {

            std::list <EdgePoint>::const_iterator itEdge;
            for(itEdge = listEdges.begin();
                itEdge != listEdges.end();
                itEdge++) {

                center   += itEdge->point;
                square_x += itEdge->point.x * itEdge->point.x;
                square_y += itEdge->point.y * itEdge->point.y;

            }

            const size_t nSz = listEdges.size();

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


    cv::Point2f get_edge_mean(std::list<EdgePoint> & pointList)
    {
        double sumx = 0, sumy = 0;
        cv::Point2f edge_mean(0, 0);

        std::list<EdgePoint>::iterator edge;
        for(edge = pointList.begin(); edge != pointList.end(); edge++) {
            sumx += edge->point.x;
            sumy += edge->point.y;
        }

        if (pointList.size() != 0) {
            edge_mean.x = sumx / (double)pointList.size();
            edge_mean.y = sumy / (double)pointList.size();
        } else {
            edge_mean.x = -1;
            edge_mean.y = -1;
        }

        return edge_mean;
    }


} // end of "namespace Burst"

