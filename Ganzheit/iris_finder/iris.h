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

#ifndef IRIS_H
#define IRIS_H

#include <list>
#include <opencv2/core/core.hpp>



namespace iris {

    /*
     * Run the starburst algorithm and compute the threshold, the ROI etc.
     * 1. If sp is defined, it will be used as the starting point for the algorithm.
     * 2. If sp is not defined the last start point will be used, with the exception
     *    that the centre of the image will be used when reset has been initiated.
     */
    bool burst(const cv::Mat &img_gray,
               const cv::Rect &roi,
               const cv::RotatedRect &ellipse,
               std::vector<cv::Point2f> &listEdges);


} // end of "namespace iris"


#endif

