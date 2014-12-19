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

#include "starburst.h"
#include "trackerSettings.h"

static const double PI = 3.141592653589;
static const int EDGE_THR_MIN = 5;


Starburst::Starburst(void)
{
	this->b_reset = true;
}

bool Starburst::process(const cv::Mat &img_gray, const cv::Rect &roi, cv::Point2f sp)
{

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


	ProcessStatus ret = tryProcess(img_gray, validRoi, sp);

	if(ret == RECOVERABLE_ERROR) {
		ret = tryProcess(img_gray, validRoi, sp);
	}

	if(ret == OK) {
		return true;
	}

	return false;
}

ProcessStatus Starburst::tryProcess(const cv::Mat &img_gray, const cv::Rect &roi, cv::Point2f sp) {

	// Clear old results
	edge_point.clear();
	edge_point_outlier.clear();


    if(sp.x != -1 && sp.y != -1) {
        start_point = sp;
    }

    if(start_point.x < roi.x || start_point.x > roi.x + roi.width  - 1 ||
       start_point.y < roi.y || start_point.y > roi.y + roi.height - 1) {

        start_point.x = roi.x + (roi.width  >> 1);
        start_point.y = roi.y + (roi.height >> 1);

    }


	if(this->b_reset) {

		int threshold;
		if(!this->testThresholds(img_gray,
                                 roi,
                                 threshold,
                                 this->varianceOfOriginalImage,
                                 sp)) {

			return UNRECOVERABLE_ERROR;
		}

		this->pupil_edge_thres = threshold;

	}

	if(!this->starburst_pupil_contour_detection(img_gray,
                                                roi,
                                                this->pupil_edge_thres,
                                                trackerSettings.STARBURST_CIRCULAR_STEPS,
                                                trackerSettings.STARBURST_MIN_SEED_POINTS)) {

		ProcessStatus ret = this->b_reset ? UNRECOVERABLE_ERROR : RECOVERABLE_ERROR;
		this->b_reset = true;
		return ret;
	}

	cv::Point2f center;
	double var = calculateStatistics(center);

	if(var < 0 || !edge_point.size()) {

		ProcessStatus ret = this->b_reset ? UNRECOVERABLE_ERROR : RECOVERABLE_ERROR;
		this->b_reset = true;
		return ret;
	}

    removeOutliers(center, var);

	var = calculateStatistics(center);
	if(var < 0 || !edge_point.size()) {
		this->pupilVariance = 0;
		ProcessStatus ret = this->b_reset ? UNRECOVERABLE_ERROR : RECOVERABLE_ERROR;
		this->b_reset = true;
		return ret;
	}

	this->pupilVariance = var;

	double sd_difference = std::abs(std::sqrt(var) - std::sqrt(varianceOfOriginalImage));

	if(var < 0 || !edge_point.size() || 
		(!b_reset && sd_difference > std::sqrt(varianceOfOriginalImage) * 
		trackerSettings.STARBURST_RELATIVE_MAX_POINT_VARIANCE)) {

		ProcessStatus ret = this->b_reset ? UNRECOVERABLE_ERROR : RECOVERABLE_ERROR;
		this->b_reset = true;
		return ret;
	}

	ROICenter = center;
	this->start_point = center;

	this->image_threshold = this->computeThreshold(img_gray);
	this->b_reset = false;

	return OK;
}


bool Starburst::testThresholds(const cv::Mat &img_gray,
                               const cv::Rect &roi,
                               int &threshold,
                               double &pointVariance,
                               cv::Point2f suggestedStartPoint) {

    // was a valid point given
    bool bStartPointSet = suggestedStartPoint.x != -1 && suggestedStartPoint.y != -1;

	double lowest_var = -1;

	// Test different threshold values
	int best_threshold = 1;
	int numOfPoints = -1;

	cv::Point2f best_point(0, 0);

    const int nX = bStartPointSet ? 1 : (int)trackerSettings.STARBURST_BLOCK_COUNT;
    const int nY = nX;


	// Try different starting points
	for(int x = 1; x <= nX; x++) {

		start_point.x = bStartPointSet        ?
                        suggestedStartPoint.x : 
                        roi.x + x * (roi.width / ((int)trackerSettings.STARBURST_BLOCK_COUNT + 1));

		for(int y = 1; y <= nY; y++) {

            start_point.y = bStartPointSet        ?
                            suggestedStartPoint.y : 
                            roi.y + y * (roi.height / ((int)trackerSettings.STARBURST_BLOCK_COUNT + 1));


			// Try different thresholds
			for(pupil_edge_thres = EDGE_THR_MIN;
				pupil_edge_thres < 40;
				pupil_edge_thres++) {

				if(!this->starburst_pupil_contour_detection(
                                        img_gray,
                                        roi,
                                        this->pupil_edge_thres,
                                        trackerSettings.STARBURST_CIRCULAR_STEPS,
                                        trackerSettings.STARBURST_MIN_SEED_POINTS)) {

					continue;

				}

				cv::Point2f center;
				double var = calculateStatistics(center);

				/* If there happens to be a good threshold
				 * with a smaller amount of points, pick it
				 * (avoid selecting "almost perfect"
				 * threshold value) */

				if((var >= 0) && (lowest_var < 0 ||
					std::sqrt(var) < std::sqrt(lowest_var))) {

					if((numOfPoints > (int)edge_point.size()) || (numOfPoints == -1)) {
						numOfPoints = this->edge_point.size();
						lowest_var = var;
						best_threshold = pupil_edge_thres;
						best_point = start_point;
					}

				}

			}
		}
	}

	if(lowest_var < 0) {
		return false;
	}


	threshold = best_threshold;
	pointVariance = lowest_var;
	start_point = best_point;
	

	return true;
}


//------------ Starburst pupil edge detection -----------//

// Input
// pupil_image: input image
// width, height: size of the input image
// cx,cy: central start point of the feature detection process
// pupil_edge_threshold: best guess for the pupil contour threshold 
// N: number of rays 
// minimum_candidate_features: must return this many features or error


bool Starburst::starburst_pupil_contour_detection(const cv::Mat &img_gray,
                                                  const cv::Rect &roi,
                                                  int edge_thresh,
                                                  int N,
                                                  unsigned int minimum_cadidate_features) {

	const int dis = 7;
    //const double angle_spread = PI;
    const double angle_spread = PI*0.8;

	//const double angle_step = (2 * PI) / (double)N;
    const double angle_step = angle_spread / (double)(N - 1);

	double cx = start_point.x;
	double cy = start_point.y;

	int loop_count = 0;

	while(loop_count <= trackerSettings.STARBURST_MAX_ITERATIONS) {

		edge_point.clear();

		/*************************************************************
		 * First phase of the algorithm: Find the seed points
		 ************************************************************/

		pointContainer seed_points;
        //Starburst::locate_edge_points(img_gray, roi, cx, cy, dis, angle_step, 0, 2 * PI, edge_thresh, seed_points);
        Starburst::locate_edge_points(img_gray, roi, cx, cy,
                                      dis, (2.0 * PI) / (double)(N-1), 0, 2.0 * PI,
                                      edge_thresh, seed_points);


		// Is there enough seed points?
		if (seed_points.size() < minimum_cadidate_features) {

			// Store the found points for debugging purposes and return.
			edge_point = seed_points;
			break;
		}

		/*************************************************************
		 * Second phase of the algorithm: Find the points from
		 * returning rays.
		 ************************************************************/

		pointContainer::iterator itEdge;
		for(itEdge = seed_points.begin(); itEdge != seed_points.end(); itEdge++) {

            double angle_normal = atan2(cy - itEdge->point.y, cx - itEdge->point.x);

            /*
             * TODO: The new step is less than or equal to the original one.
             * When it's less than the original, the algorithm does not cover
             * the entire spread. Just from [min, max-(orig-new)]. Rethink.
             */
            double new_angle_step = angle_step * ( (double)edge_thresh / (double)itEdge->intensityDifference);

			Starburst::locate_edge_points(img_gray,
                                          roi,
                                          itEdge->point.x,
                                          itEdge->point.y,
                                          dis,
                                          new_angle_step,
                                          angle_normal,
                                          angle_spread,
                                          edge_thresh,
                                          edge_point);

		}

		/*************************************************************
		 * Process the results
		 ************************************************************/

		// Merge seed points with edge_points.
		//edge_point.splice(edge_point.begin(), seed_points);
        edge_point.insert(edge_point.end(), seed_points.begin(), seed_points.end());

		loop_count += 1;
		cv::Point2f edge_mean = get_edge_mean(edge_point);

		// Converge?
		if ((fabs(edge_mean.x-cx) + fabs(edge_mean.y-cy)) < trackerSettings.STARBURST_REQUIRED_ACCURACY) {

			return true;
		}

		cx = edge_mean.x;
		cy = edge_mean.y;

	}

	return false;
}


// static method
void Starburst::locate_edge_points(const cv::Mat &img_gray,
                                   const cv::Rect &roi,
                                   double cx, double cy,
                                   int dis, double angle_step, double angle_normal, double angle_spread,
                                   int edge_thresh, pointContainer & pointList) {

	double angle;
	cv::Point2f p;
	double dis_cos, dis_sin;
	int pixel_value1, pixel_value2;

    // inclusive bounds
    const int xEnd = roi.x + roi.width - 1;
    const int yEnd = roi.y + roi.height - 1;

	for (angle = angle_normal - angle_spread / 2.0 + 0.0001; angle < angle_normal + angle_spread / 2.0; angle += angle_step) {
		dis_cos = dis * cos(angle);
		dis_sin = dis * sin(angle);
		p.x = cx + dis_cos;
		p.y = cy + dis_sin;

		if (p.x < roi.x || p.x > xEnd || p.y < roi.y || p.y > yEnd)
			continue;

		pixel_value1 = img_gray.data[(int)(p.y)*img_gray.step+(int)(p.x)];
		while (1) {
			p.x += dis_cos;
			p.y += dis_sin;
			if (p.x < roi.x || p.x > xEnd || p.y < roi.y || p.y > yEnd)
				break;

			pixel_value2 = img_gray.data[(int)(p.y)*img_gray.step+(int)(p.x)];

			if (pixel_value2 - pixel_value1 > edge_thresh) {
				pointList.push_back(EdgePoint(cv::Point2f(p.x - dis_cos / 2.0, p.y - dis_sin / 2.0), pixel_value2 - pixel_value1));
				break;
			}

			pixel_value1 = pixel_value2;
		}
	}
}


unsigned int Starburst::computeThreshold(const cv::Mat &img_gray)
{
	unsigned char *data = (unsigned char *)img_gray.data;
	int step = img_gray.step;

	size_t n = this->edge_point.size();
	unsigned int sum = 0;

	pointContainer::iterator edge;

	for(edge = edge_point.begin(); edge != edge_point.end(); edge++) {
		int ind = round(edge->point.y) * step + round(edge->point.x);

		sum += data[ind];
	}

	return (unsigned char)((double)sum / (double)n + 0.5);
}

void Starburst::removeOutliers(cv::Point2f &mean, double var)
{

	const double stdDev = std::sqrt(var);	

	pointContainer::iterator edge;
	for(edge = edge_point.begin(); edge != edge_point.end();) {

		double diff_x = edge->point.x - mean.x;
		double diff_y = edge->point.y - mean.y;

		if(std::sqrt(diff_x * diff_x + diff_y * diff_y) > (stdDev * 
			trackerSettings.STARBURST_OUTLIER_DISTANCE)) {

			// Store the element
			edge_point_outlier.push_back(*edge);

			// Take the element to remove
			pointContainer::iterator remove_pos = edge;

			// Is this the first element?
			if(remove_pos != edge_point.begin()) {
				// No.. just remove it
				edge--;
				edge_point.erase(remove_pos);
			} else {
				// We're removing the first element... remove and reset iterator.
				edge_point.erase(remove_pos);
				edge = edge_point.begin();
				continue;
			}
		}

		// Next element (NOTE! This must not be run if we are dealing with the first element!)
		edge++;
	}
}


double Starburst::calculateStatistics(cv::Point2f &center)
{
	double square_x = 0, square_y = 0;

	center = cv::Point2f(0.0, 0.0);

	if(edge_point.size() != 0) {

		pointContainer::const_iterator itEdge;
		for(itEdge = edge_point.begin();
			itEdge != edge_point.end();
			itEdge++) {

			center   += itEdge->point;
			square_x += itEdge->point.x * itEdge->point.x;
			square_y += itEdge->point.y * itEdge->point.y;

		}

		center.x /= edge_point.size();
		center.y /= edge_point.size();

		square_x /= edge_point.size();
		square_y /= edge_point.size();

		double var_x = (center.x * center.x - square_x);
		double var_y = (center.y * center.y - square_y);

		return std::sqrt(var_x * var_x + var_y * var_y);

	}

	return -1;
}


cv::Point2f Starburst::get_edge_mean(pointContainer & pointList)
{
	double sumx = 0, sumy = 0;
	cv::Point2f edge_mean(0, 0);

	pointContainer::iterator edge;
	for(edge = pointList.begin(); edge != pointList.end(); edge++) {
		sumx += edge->point.x;
		sumy += edge->point.y;
	}

	if (edge_point.size() != 0) {
		edge_mean.x = sumx / (double)edge_point.size();
		edge_mean.y = sumy / (double)edge_point.size();
	} else {
		edge_mean.x = -1;
		edge_mean.y = -1;
	}

	return edge_mean;
}


