#include "LEDCalibPattern.h"
#include <stdio.h>
#include "imgproc.h"
#include <sstream>


namespace calib {

    namespace LEDCalibPattern {

        static std::string s_strErr;

        std::string getLastError() {
            return s_strErr;
        }


        class DIST {
        public:
            DIST(int _ind, double _dist) {
                ind = _ind;
                dist = _dist;
            }

            int ind;
            double dist;
        };
        struct sortdist {
            bool operator() (DIST a, DIST b) {return a.dist < b.dist;}
        } sortdist;


        static bool cmp_is_equal(unsigned char val, unsigned char th) {
            return val == th;
        }



        static bool getRectCorners(const cv::Mat &img_gray,
                                   const std::vector<std::vector<cv::Point> > &contours,
                                   std::vector<cv::Point2f> &corners);


        /*
         * Get the corner circles
         */
        static bool getCircleCorners(cv::Mat &img_binary,
                                     const std::vector<std::vector<cv::Point> > &contours,
                                     const std::vector<cv::Point2f> coms,
                                     const std::vector<cv::Point2f> &corners,
                                     std::vector<cv::RotatedRect> &ellipses,
                                     std::vector<int> &ind_ellipses,
                                     cv::vector<cv::Point2f> &image_points);

        static double pointLineDistance(const cv::Point2f &p, const cv::Point2f line[2]);


        static int getClosestCOM(const cv::Point &p, const std::vector<cv::Point2f> &coms);

        static bool getOtherPoints(const cv::Mat img_binary,
                                   const std::vector<std::vector<cv::Point> > &contours,
                                   const std::vector<cv::Point2f> &coms,
                                   const std::vector<cv::RotatedRect> &corner_ellipses,
                                   const std::vector<int> & ind_corners,
                                   cv::vector<cv::Point2f> &image_points);




        void getLEDObjectPoints(double circleSpacing, std::vector<cv::Point3f> &objectPoints) {

            objectPoints.resize(16);

            double dx = circleSpacing;
            double dy = circleSpacing;


            // 0..4 and 8..12
            for(int i = 0; i < 5; ++i) {
                objectPoints[i]   = cv::Point3f(-2*dx, -2*dy + i*dy, 0);
                objectPoints[8+i] = cv::Point3f( 2*dx,  2*dy - i*dy, 0);
            }

            // 5..7 and 13..15
            for(int i = 0; i < 3; ++i) {
                objectPoints[5+i]  = cv::Point3f(-dx + i*dx,  2*dy, 0);
                objectPoints[13+i] = cv::Point3f( dx - i*dx, -2*dy, 0);
            }

        }


        void computeNormal(const Eigen::Matrix4d &transformation,
                           Eigen::Vector3d &normal) {

            // transform the centre
            Eigen::Vector4d centreNoTrasformation(0, 0, 0, 1);
            Eigen::Vector4d c = transformation * centreNoTrasformation;

            // transform the normal
            Eigen::Vector4d normalNoTransformation(0, 0, -1, 1);
            Eigen::Vector4d n = transformation * normalNoTransformation;

            normal(0) = n(0) - c(0);
            normal(1) = n(1) - c(1);
            normal(2) = n(2) - c(2);

            // no need to normalise, look at n and c
            //normal.normalize();

        }


        /*
         *
         *     o    o    o    o    o
         *        _______________
         *     o  |             |  o
         *        |             |
         *     o  |             |  o
         *        |             |
         *     o  |_____________|  o
         *
         *     o    o    o    o    o
         *
         */
        bool findMarkers(const cv::Mat &img_gray,
                         unsigned char th,
                         cv::vector<cv::Point2f> &image_points) {

            //            getLEDObjectPoints(circleSpacing, object_points);

            // resize the array to the proper size
            image_points.clear();
            image_points.resize(16);

            // create a binary image, white and black are inverted
            cv::Mat img_binary;
            cv::threshold(img_gray, img_binary, th, 255, cv::THRESH_BINARY_INV);


            // define the contours
            std::vector<std::vector<cv::Point> > contours;

            {
                // find the contours, use a temp image because it will be modified in cv::findContours()
                cv::Mat tmp = img_binary.clone();
                cv::findContours(tmp, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);
            }

            // 16 object points + the rectangle
            if(contours.size() < 16 + 1) {
                //                printf("LEDCalibPattern::findMarkers(): at least 17 contours needed, found %d\n", (int)contours.size());
                std::stringstream ss;
                ss << "LEDCalibPattern::findMarkers(): at least 17 contours needed, found " << contours.size();
                s_strErr = ss.str();
                return false;
            }


            // find the rectangle corners
            std::vector<cv::Point2f> corners;
            bool b_found = LEDCalibPattern::getRectCorners(img_gray,
                                                           contours,
                                                           corners);

            if(!b_found) {
                return false;
            }


            // compute the mass centres to the contours
            std::vector<cv::Point2f> coms(contours.size());

            for(size_t i = 0; i < contours.size(); ++i) {
                imgproc::getCOM(contours[i], coms[i]);
            }


            /*****************************************************************
             * Finding the circle corners
             *****************************************************************/
            std::vector<cv::RotatedRect> corner_ellipses;
            std::vector<int> ind_corners;

            if(!LEDCalibPattern::getCircleCorners(img_binary,
                                                  contours,
                                                  coms,
                                                  corners,
                                                  corner_ellipses,
                                                  ind_corners,
                                                  image_points)) {

                return false;

            }


            /*****************************************************************
             * Find the other circles
             *****************************************************************/
            if(!LEDCalibPattern::getOtherPoints(img_binary,
                                                contours,
                                                coms,
                                                corner_ellipses,
                                                ind_corners,
                                                image_points)) {

                return false;
            }

            return true;

        }


        // http://softsurfer.com/Archive/algorithm_0102/algorithm_0102.htm
        double pointLineDistance(const cv::Point2f &p, const cv::Point2f line[2]) {

            const cv::Point &p0 = line[0];
            const cv::Point &p1 = line[1];

            const double num = std::abs( (p0.y-p1.y)*p.x + (p1.x-p0.x)*p.y + (p0.x*p1.y - p1.x*p0.y) );
            const double den = std::sqrt( (p1.x-p0.x)*(p1.x-p0.x) + (p1.y-p0.y)*(p1.y-p0.y) );

            return num / den;

        }




        typedef bool(*three_sort)(cv::Point2f a, cv::Point2f b);
        bool sort_up_first(cv::Point2f a, cv::Point2f b) {return a.y < b.y;}
        bool sort_left_first(cv::Point2f a, cv::Point2f b) {return a.x < b.x;}
        bool sort_bottom_first(cv::Point2f a, cv::Point2f b) {return a.y > b.y;}
        bool sort_right_first(cv::Point2f a, cv::Point2f b) {return a.x > b.x;}



        bool getOtherPoints(const cv::Mat img_binary,
                            const std::vector<std::vector<cv::Point> > &contours,
                            const std::vector<cv::Point2f> &coms,
                            const std::vector<cv::RotatedRect> &corner_ellipses,
                            const std::vector<int> & ind_corners,
                            cv::vector<cv::Point2f> &image_points) {

            three_sort sort_fnc[4] = {&sort_up_first,
                                      &sort_left_first,
                                      &sort_bottom_first,
                                      &sort_right_first};

            for(int ccorners = 0; ccorners < 4; ++ccorners) {

                int corner_ind1 = ccorners;
                int corner_ind2 = (ccorners + 1) % 4;

                std::vector<DIST> dist;
                cv::Point2f line[2] = {corner_ellipses[corner_ind1].center, corner_ellipses[corner_ind2].center};
                for(size_t i = 0; i < coms.size(); ++i) {
                    dist.push_back(DIST(i, pointLineDistance(coms[i], line)));
                }
                std::sort(dist.begin(), dist.end(), sortdist);

                std::vector<cv::Point2f> three_points;
                for(size_t i = 0; three_points.size() < 3 && i < dist.size(); ++i) {

                    const cv::Point2f &p1 = coms[ind_corners[corner_ind1]];
                    const cv::Point2f &p2 = coms[ind_corners[corner_ind2]];

                    float minx = std::min(p1.x, p2.x);
                    float maxx = std::max(p1.x, p2.x);

                    float miny = std::min(p1.y, p2.y);
                    float maxy = std::max(p1.y, p2.y);

                    int ind = dist[i].ind;

                    const cv::Point2f &cur_p = coms[ind];

                    if(cur_p != p1 && cur_p != p2 &&
                       cur_p.x >= minx && cur_p.x <= maxx &&
                       cur_p.y >= miny && cur_p.y <= maxy) {

                        const std::vector<cv::Point> &curContour = contours[ind];
                        if(curContour.size() > 5) {
                            cv::RotatedRect ell = cv::fitEllipse(curContour);

                            cv::Rect rb = ell.boundingRect();
                            if(rb.x >= 0 && rb.x + rb.width -1 < img_binary.cols && rb.y >= 0 && rb.y + rb.height - 1 < img_binary.rows) {
                                three_points.push_back(ell.center);
                            }

                        }


                    }

                }

                if(three_points.size() != 3) {

                    //                    printf("LEDCalibPattern::getOtherPoints(): could not find 3 points between the corners\n");
                    std::stringstream ss;
                    ss << "LEDCalibPattern::getOtherPoints(): could not find 3 points between the corners";
                
                    s_strErr = ss.str();

                    return false;
                }

                std::sort(three_points.begin(), three_points.end(), sort_fnc[ccorners]);
                image_points[4*ccorners+1] = three_points[0];
                image_points[4*ccorners+2] = three_points[1];
                image_points[4*ccorners+3] = three_points[2];

            }


            return true;
        }


        bool getCircleCorners(cv::Mat &img_binary,
                              const std::vector<std::vector<cv::Point> > &contours,
                              const std::vector<cv::Point2f> coms,
                              const std::vector<cv::Point2f> &corners,
                              std::vector<cv::RotatedRect> &ellipses,
                              std::vector<int> &ind_ellipses,
                              cv::vector<cv::Point2f> &image_points) {

            // start and end points of lines where the corner circles are searched for
            std::vector<cv::Point2f> start(4);
            std::vector<cv::Point2f> end(4);


            // compute the end points in such a way that they lie on the diagonals
            for(int i = 0; i < 2; ++i) {

                double diag_xs = corners[i].x;
                double diag_ys = corners[i].y;
                double diag_xe = corners[i+2].x;
                double diag_ye = corners[i+2].y;

                double slope = (diag_ye-diag_ys) / (diag_xe-diag_xs);

                // distance between start and end
                const int dist = 200;

                /*************************************************************
                 * We know the starting point, (x1, y1) and the slope. We need
                 * to solve x2 and y2 with a given distance. The definition of
                 * the slope is "how much up with 1 step to the right".
                 *
                 *       y1 = a*x1 + b
                 *       y2 = d*a + y1
                 *
                 *    where d is the number of x steps
                 *
                 *       sqrt(d² + (d*a)² = dist
                 *
                 *       d = dist / sqrt(1 + a²)
                 *
                 *************************************************************/
                double dx = dist / std::sqrt(1.0 + slope*slope);

                /*
                 * Put (x1 y1) a bit further away from the corner, so that it does not
                 * interfere with the detection
                 */
                start[i].x = corners[i].x - 4;
                start[i].y = corners[i].y - 4*slope;

                start[i+2].x = corners[i+2].x + 4;
                start[i+2].y = corners[i+2].y + 4*slope;


                end[i].x = -dx + corners[i].x;
                end[i].y = -dx * slope + corners[i].y;

                end[i+2].x = dx + corners[i+2].x;
                end[i+2].y = dx * slope + corners[i+2].y;

            }


            for(int i = 0; i < 4; ++i) {

                const int img_w = img_binary.cols;
                const int img_h = img_binary.rows;

                // no need to check end[i] because, setEndPoints() does that
                if(start[i].x < 0      || start[i].y < 0      ||
                   start[i].x >= img_w || start[i].y >= img_h) {

                    return false;

                }

                const int xs = start[i].x;
                const int ys = start[i].y;
                int xe = end[i].x;
                int ye = end[i].y;


                imgproc::setEndPoints(xs,
                                      ys,
                                      xe,
                                      ye,
                                      xe - xs,
                                      ye - ys,
                                      img_w,
                                      img_h);


                if(xs < 0 || xs >= img_binary.cols || ys < 0 || ys >= img_binary.rows) {
                    //                    printf("LEDCalibPattern::getCircleCorners(): (xs, ys) not ok (%d, %d)\n", xs, ys);
                    std::stringstream ss;
                    ss << "LEDCalibPattern::getCircleCorners(): (xs, ys) not ok " << xs << "(" << ", " << ys << ")";
                    s_strErr = ss.str();
                    return false;
                }

                if(xe < 0 || xe >= img_binary.cols || ye < 0 || ye >= img_binary.rows) {
                    //                    printf("LEDCalibPattern::getCircleCorners(): (xe, ye) not ok (%d, %d)\n", xe, ye);
                    std::stringstream ss;
                    ss << "LEDCalibPattern::getCircleCorners(): (xe, ye) not ok " << xe << "(" << ", " << ye << ")";
                    s_strErr = ss.str();
                    return false;
                }

                cv::Point ret_binary_black;
                cv::Point ret_binary_white;

                imgproc::findFromLine(img_binary,
                                      round(xs),
                                      round(ys),
                                      round(xe),
                                      round(ye),
                                      255,
                                      ret_binary_black,
                                      ret_binary_white,
                                      cmp_is_equal);


                if(ret_binary_black.x == -1) {
                    //                    printf("LEDCalibPattern::getCircleCorners(): %dth circle not on line\n", i);
                    std::stringstream ss;
                    ss << "LEDCalibPattern::getCircleCorners(): " << i << "th circle not on line";
                    s_strErr = ss.str();
                    return false;
                }


                int ind_closest = LEDCalibPattern::getClosestCOM(ret_binary_black, coms);
                ind_ellipses.push_back(ind_closest);


                /*
                 * Ellipse fitting
                 */
                const std::vector<cv::Point> &curContour = contours[ind_closest];
                if(curContour.size() < 5) {

                    //                    printf("LEDCalibPattern::getCircleCorners(): ellipse fitting needs at leas 5 points\n");

                    std::stringstream ss;
                    ss << "LEDCalibPattern::getCircleCorners(): ellipse fitting needs at leas 5 points";
                    s_strErr = ss.str();

                    return false;

                }
                cv::RotatedRect ell = cv::fitEllipse(curContour);

                // sanity check
                cv::Rect rb = ell.boundingRect();
                if(rb.x < 0 || rb.x + rb.width >= img_binary.cols - 1 || rb.y < 0 || rb.y + rb.height >= img_binary.rows - 1) {

                    // printf("LEDCalibPattern::getCircleCorners(): ellipse not ok (%.2f, %.2f, %.2f, %.2f)\n",
                    //        ell.center.x,
                    //        ell.center.y,
                    //        ell.size.width,
                    //        ell.size.height);

                    std::stringstream ss;
                    ss << "LEDCalibPattern::getCircleCorners(): ellipse not ok " << "(" << ell.center.x
                       << ", " << ell.center.y << ", " << ell.size.width << ", " << ell.size.height << ")";

                    s_strErr = ss.str();

                    return false;

                }

                ellipses.push_back(ell);

            }


            for(int i = 0; i < 4; ++i) {
                image_points[4*i] = ellipses[i].center;
            }

            return true;

        }


        int getClosestCOM(const cv::Point &p,
                          const std::vector<cv::Point2f> &coms) {

            // see to which contour the point belongs
            double dist_min = 1e9;
            int ind = 0;

            for(size_t i = 0; i < coms.size(); ++i) {

                double x1 = coms[i].x;
                double y1 = coms[i].y;

                double x2 = p.x;
                double y2 = p.y;

                double dx = x2 - x1;
                double dy = y2 - y1;

                double dist2 = dx*dx + dy*dy;

                if(dist2 < dist_min) {
                    dist_min = dist2;
                    ind = i;
                }

            }


            return ind;
        }

        bool myfnc_left_first(cv::Point2f i, cv::Point2f j);
        void myorganise_corners(std::vector<cv::Point2f> &corners);

        bool getRectCorners(const cv::Mat &img_gray,
                            const std::vector<std::vector<cv::Point> > &contours,
                            std::vector<cv::Point2f> &corners) {

            /***********************************************
             * Copy-pasted from samples/cpp/squares.cpp
             ***********************************************/

            std::vector<std::vector<cv::Point> > squares;


            /***********************************************
             * Erode and dilate the image
             ***********************************************/

            const unsigned int MIN_AREA = 100;

            unsigned int cBadContourArea  = 0;
            unsigned int cNotConvex       = 0;
            unsigned int cCornersTooSmall = 0;

            // test each contour
            for(size_t i = 0; i < contours.size(); i++) {

                std::vector<cv::Point> approx;

                // approximate contour with accuracy proportional
                // to the contour perimeter
                cv::approxPolyDP(cv::Mat(contours[i]), approx, cv::arcLength(cv::Mat(contours[i]), true)*0.02, true);

                // square contours should have 4 vertices after approximation
                // relatively large area (to filter out noisy contours)
                // and be convex.
                // Note: absolute value of an area is used because
                // area may be positive or negative - in accordance with the
                // contour orientation

                if(approx.size() != 4) {
                    continue;
                }

                if(fabs(cv::contourArea(cv::Mat(approx))) <= MIN_AREA) {
                    ++cBadContourArea;
                    continue;
                }


                /*
                 * For some reason this convexity check fails quite oftern, therefore
                 * it had to be commented out.
                 */
                // if(cv::isContourConvex(cv::Mat(approx))) {
                //     ++cNotConvex;
                //     continue;
                // }


                double maxCosine = 0;

                for(int j = 2; j < 5; j++) {
                    // find the maximum cosine of the angle between joint edges
                    double cosine = fabs(imgproc::angle(approx[j%4], approx[j-2], approx[j-1]));
                    maxCosine = MAX(maxCosine, cosine);
                }

                // if cosines of all angles are small
                // (all angles are ~90 degree) then write quandrange
                // vertices to resultant sequence. Also the x coordinates of all
                // corners must not lie very close to the edge
                if(maxCosine >= 0.5) {
                    ++cCornersTooSmall;
                    continue;
                }

                if(approx[0].x > 1 && approx[1].x > 1 && approx[2].x > 1 && approx[3].x > 1) {
                    squares.push_back(approx);
                }

            }

            // if no contours were found
            if(squares.size() == 0) {

                // printf("LEDCalibPattern::getRectCorners(): bad contour areas: %d, not convex: %d, small corners: %d\n",
                //        cBadContourArea,
                //        cNotConvex,
                //        cCornersTooSmall);


                std::stringstream ss;
                ss << "LEDCalibPattern::getRectCorners(): bad contour areas: " << cBadContourArea << ", not convex: " << cNotConvex << ", small corners: " << cCornersTooSmall;

                s_strErr = ss.str();

                return false;
            }

            /*
             *	Usually when squares.size() > 1, two contours have been found: The inner
             *	and the outer. The outer one is what we are looking for and its area is
             *	greater than the inner one's.
             */
            std::vector<cv::Point> *c;
            if(squares.size() > 1) {
                double area1 = fabs(cv::contourArea(cv::Mat(squares[0])));
                double area2 = fabs(cv::contourArea(cv::Mat(squares[1])));
                c = area1 > area2 ? &squares[0] : &squares[1];
            }
            else {	// only 1 contour was found
                c = &squares[0];
            }

            corners.push_back((*c)[0]);
            corners.push_back((*c)[1]);
            corners.push_back((*c)[2]);
            corners.push_back((*c)[3]);

            cv::cornerSubPix(img_gray,
                             corners,
                             cv::Size(15, 15),
                             cv::Size(-1, -1),
                             cv::TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));


            myorganise_corners(corners);

            return true;
        }



        void makeTransformationMatrix(const cv::Mat &cv_intr,
                                      const cv::Mat &cv_dist,
                                      const std::vector<cv::Point2f> &image_points,
                                      const std::vector<cv::Point3f> &object_points,
                                      Eigen::Matrix4d &transformation) {


			/************************************************************
			 * Solve the transformation.
			 *    The OpenCV matrix is defined in the following way:
			 *       x+ : right
			 *       x- : left
			 *       y+ : down
			 *       y- : up
			 *       z+ : towards the monitor
			 *       z- : towards the camera
			 ************************************************************/

			cv::Mat rvec, tvec;
			cv::solvePnP(object_points, image_points, cv_intr, cv_dist, rvec, tvec);

			cv::Mat rmat;
			cv::Rodrigues(rvec, rmat);

		
            // Copy cv::Mat to Eigen::Matrix4d

			// row 1
			transformation(0, 0) = rmat.at<double>(0, 0);
			transformation(0, 1) = rmat.at<double>(0, 1);
			transformation(0, 2) = rmat.at<double>(0, 2);
			transformation(0, 3) = tvec.at<double>(0);

			// row 2
			transformation(1, 0) = rmat.at<double>(1, 0);
			transformation(1, 1) = rmat.at<double>(1, 1);
			transformation(1, 2) = rmat.at<double>(1, 2);
			transformation(1, 3) = tvec.at<double>(1);

			// row 3
			transformation(2, 0) = rmat.at<double>(2, 0);
			transformation(2, 1) = rmat.at<double>(2, 1);
			transformation(2, 2) = rmat.at<double>(2, 2);
			transformation(2, 3) = tvec.at<double>(2);

			// row 4
			transformation(3, 0) = 0;
			transformation(3, 1) = 0;
			transformation(3, 2) = 0;
			transformation(3, 3) = 1;

		}





        bool myfnc_left_first(cv::Point2f i, cv::Point2f j) {return (i.x < j.x);}


        /*
         *        0 ___________ 3
         *          |         |
         *          |         |
         *          |         |
         *          |_________|
         *        1             2
         *
         */
        void myorganise_corners(std::vector<cv::Point2f> &corners) {

            // get the two left corners
            std::sort(corners.begin(), corners.end(), myfnc_left_first);
            cv::Point2f lup		= corners[0].y < corners[1].y ? corners[0] : corners[1];
            cv::Point2f ldown	= corners[0].y > corners[1].y ? corners[0] : corners[1];

            // get the two right corners
            cv::Point2f rup		= corners[2].y < corners[3].y ? corners[2] : corners[3];
            cv::Point2f rdown	= corners[2].y > corners[3].y ? corners[2] : corners[3];

            // re-assign the vector
            corners.clear();
            corners.push_back(lup);
            corners.push_back(ldown);
            corners.push_back(rdown);
            corners.push_back(rup);
        }


    }

}

