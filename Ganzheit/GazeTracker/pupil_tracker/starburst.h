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

#ifndef _STARBURST_H
#define _STARBURST_H

//#ifndef PI
//#define PI 3.141592653589
//#endif


#include <list>
#include <opencv2/imgproc/imgproc.hpp>



enum ProcessStatus {OK, RECOVERABLE_ERROR, UNRECOVERABLE_ERROR};

class EdgePoint {

public:	

    EdgePoint(cv::Point2f _point, int _intensity) {

        this->point = _point;
        this->intensityDifference = _intensity;

    }

    cv::Point2f point;
    int intensityDifference;
};

typedef std::vector<EdgePoint> pointContainer;


class Starburst {

public:

    Starburst(void);

    void reset(void) {
        this->b_reset = true;
    }


    /*
     * Run the starburst algorithm and compute the threshold, the ROI etc.
     * 1. If sp is defined, it will be used as the starting point for the algorithm.
     * 2. If sp is not defined the last start point will be used, with the exception
     *    that the centre of the image will be used when reset has been initiated.
     */
    bool process(const cv::Mat &img_gray, const cv::Rect &roi = cv::Rect(), cv::Point2f sp = cv::Point2f(-1, -1));

    const cv::Point2f getROICenter(void) const {
        if(this->b_reset)
            return cv::Point2f(-1, -1);

        return this->ROICenter;
    }
    int getThreshold(void) const {
        if(this->b_reset)
            return -1;

        return this->image_threshold;
    }
    void setStartPoint(const cv::Point2f &point) {
        this->start_point = point;
    }

    const pointContainer & getOutlierPoints(void) const {
        return this->edge_point_outlier;
    }

    const pointContainer & getInlierPoints(void) const {
        return this->edge_point;
    }

    double getPupilVariance(void) const {
        return this->pupilVariance;
    }

    cv::Point2f getStartPoint() const {
        return start_point;
    }

private:

    double calculateStatistics(cv::Point2f &center);

    bool testThresholds(const cv::Mat &img_gray,
                        const cv::Rect &roi,
                        int &threshold,
                        double &pointVariance,
                        cv::Point2f suggestedStartPoint);


    unsigned int computeThreshold(const cv::Mat &img_gray);

    bool starburst_pupil_contour_detection(const cv::Mat &img_gray,
                                           const cv::Rect &roi,
                                           int edge_thresh,
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
                                   pointContainer & pointList);

    void removeOutliers(cv::Point2f &mean, double var);
    cv::Point2f get_edge_mean(pointContainer & pointList);

    ProcessStatus tryProcess(const cv::Mat &img_gray,
                             const cv::Rect &roi,
                             cv::Point2f sp);

    int image_threshold;
    double varianceOfOriginalImage;
    int pupil_edge_thres;

    double pupilVariance;

    cv::Point2f ROICenter;
    cv::Point2f start_point;
		
    pointContainer edge_point;
    pointContainer edge_point_outlier;

    bool b_reset;

};

#endif

