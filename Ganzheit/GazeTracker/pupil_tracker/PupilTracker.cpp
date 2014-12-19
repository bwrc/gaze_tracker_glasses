#include "PupilTracker.h"
#include "trackerSettings.h"
#include "CRTemplate.h"
#include "iris.h"
#include "ellipse.h"


#include <math.h>


#define ROUND(X)		((X) > 0.0 ? ((X) + 0.5) : ((X) - 0.5))
static const double pi                      = 3.1415926535897932384626433832795;
static const double TH_FF                   = 0.7;		/* threshold for the floodfill */
static const double HUGE_ERROR_DBL          = 1e9;
static const unsigned int HUGE_ERROR_UINT   = 1e9;


#define RADTODEG(RAD)	( ( (180.0*(RAD)) / (pi) ) )
#define DEGTORAD(DEG)	( ((DEG)*(pi)) / (180.0) )



namespace gt {

    static void fitRectIntoRect(const cv::Rect &rectBig, cv::Rect &rectSmall);
    static bool sort_crs(cv::Point2d i, cv::Point2d j) {return (i.x < j.x);}


    PupilTracker::PupilTracker() {

        this->ellipse_pupil = cv::RotatedRect();

        // initially there are 2 trackable CRs
        crs.setMax(2);


        // thresholds, when adaptive th is being used th.pupil will be computed automatically
        this->thresholds.pupil	= trackerSettings.PUPIL_THRESHOLD;
        this->thresholds.cr		= trackerSettings.CR_THRESHOLD;

        /*
         * Initialise the threshold averager with a buffer size, i.e. how many
         * previous thresholds are used in averaging the current threshold.
         */
        thresholdAverager.init(3);

        // the same goes for the variance averager
        sbVarianceAverager.init(30);


        // set the last valid starburst start point
        lastValidSBPoint.x = lastValidSBPoint.y = -1;

        this->failed_tracks = 0;

        precomputeRays();

        m_rectRoi.width  = trackerSettings.ROI_W_DEFAULT;
        m_rectRoi.height = trackerSettings.ROI_H_DEFAULT;

    }


    PupilTracker::~PupilTracker() {

        this->crs.getCentres().clear();

        distX.clear();
        distY.clear();

    }


    void PupilTracker::useAutoTh(bool t) {
        trackerSettings.AUTO_THRESHOLD = t ? 1 : 0;
    }


    void PupilTracker::precomputeRays() {

        int n = trackerSettings.NOF_RAYS;
        if(n % 2 != 0) {
            printf("PupilTracker::precomputeRays(): NOF_RAYS is not even %d\n", n);
            ++n; // increase to achvieve even count
        }

        this->distX.resize(n);
        this->distY.resize(n);


        /*
         * From -45 to 45, on both sides
         */
        double dx = 2 * (pi / 2.0) / (n - 2.0);
        double ang = -45.0 * pi / 180.0;

        // half
        int n2 = n / 2;

        for(int i = 0; i < n2; ++i) {

            // right side
            this->distX[i] = (int)ROUND(trackerSettings.MAX_PUPIL_RADIUS * cos(ang));
            this->distY[i] = (int)ROUND(trackerSettings.MAX_PUPIL_RADIUS * sin(ang));

            // left side
            this->distX[n2 + i] = -this->distX[i];
            this->distY[n2 + i] = -this->distY[i];

            ang += dx;

        }

    }


    void PupilTracker::reset() {

        this->failed_tracks = 0;
        this->starburst.reset();

    }


    /*
     * Partially based on the method described in:
     *
     *   "FreeGaze: A Gaze Tracking System for Everyday Gaze Interaction".
     */
    bool PupilTracker::track(const cv::Mat &_img, const cv::Point2f *suggestedStartPoint) {

        // This does copy the data
        m_imgGray = _img.clone();

        // check that the crop area is ok and adjust if necessary
        checkCropArea();

        /*
         * Preprocess, i.e. apply histogram equalisation etc.
         */
        preprocessImage();


        clusterLabels.clear();

        /*
         * Clear previous clusters, so that when getting the clusters
         * the caller does not get the previously detected clusters.
         */
        clusteriser.clearClusters();


        /***********************************************************************
         * Perform starburst to define th ROI, if ROI was not given
         ***********************************************************************/
        if(!define_ROI(suggestedStartPoint)) {

            /***********************************************************************
             * Clear all temporary variables only here, because define_ROI()
             * depends on these variables.
             * Also important to clear the vars now so that previous results
             * are not obtainable.
             ***********************************************************************/
            //this->clearVars();
            //printf("Using default roi\n");
            //            return false;

        }


        /***********************************************************************
         * Clear all temporary variables only here, because define_ROI()
         * depends on these variables
         ***********************************************************************/
        this->clearVars();


        /***********************************************************************
         * Threshold the image. Use auto threshold if requested
         ***********************************************************************/
        if(trackerSettings.AUTO_THRESHOLD) {

            unsigned char val = this->starburst.getThreshold();
            thresholdAverager.add(val);
            thresholds.pupil = thresholdAverager.getAverage();

        }
        else {
            thresholds.pupil = trackerSettings.PUPIL_THRESHOLD;
        }

        this->thresholds.cr = trackerSettings.CR_THRESHOLD;

        // thresholding done only for the part defined by the ROI
        thresholdImage(this->thresholds.pupil, m_rectRoi);


        /***********************************************************************
         * Get clusters in the binary image within the ROI. Fill the holes so
         * that they do not disturb the ellipse fitting.
         ***********************************************************************/
        clusteriser.clusterise(m_imgBinary, m_rectRoi);

        {

            /*
             * Fill all holes with the pupil color because some of them might
             * be glints and might cause distortion if located inside the pupil.
             */
            const std::vector<Cluster> &clusters = clusteriser.getClusters();
            const std::vector<int> &ind_holes    = clusteriser.getHoleIndices();
            const size_t nOfHoles                = ind_holes.size();
            const cv::Scalar color(BINARY_BLACK);

            for(size_t i = 0; i < nOfHoles; ++i) {
                cv::drawContours(m_imgBinary, clusters, ind_holes[i], color, CV_FILLED, 8);
            }

        }


        /***********************************************************************
         * Determine which cluster is the pupil
         ***********************************************************************/
        if(getPupilFromClusters()) {

            this->starburst.setStartPoint(ellipse_pupil.center);
            this->failed_tracks = 0;


            /***********************************************************************
             * Track the eye lid boundaries within an angle. If the tracking
             * fails, it is not too serious, the corneal reflections will be
             * looked for withing a default area.
             ***********************************************************************/
            m_crSearchEllipse = trackEyeLids();


            /***********************************************************************
             * Track the corneal reflections
             ***********************************************************************/
            findCornealReflections();

            return true;

        }


        /***********************************************************************
         * The pupil was not found
         ***********************************************************************/
        this->cluster_pupil.clear();

        this->ellipse_pupil = cv::RotatedRect();

        if(failed_tracks++ > trackerSettings.MAX_FAILED_TRACKS_BEFORE_RESET) {

            failed_tracks = 0;
            this->starburst.reset();
            lastValidSBPoint.x = m_rectCrop.x + (m_rectCrop.width >> 1);
            lastValidSBPoint.x = m_rectCrop.y + (m_rectCrop.height >> 1);

        }

        return false;

    }


    bool PupilTracker::define_ROI(const cv::Point2f *suggestedStartPoint) {

        // default values
        int roiW = trackerSettings.ROI_W_DEFAULT;
        int roiH = trackerSettings.ROI_H_DEFAULT;

        /***********************************************************************
         * Suggest a size for the ROI based on the size of the previously
         * found pupil ellipse
         ***********************************************************************/
        if(cluster_pupil.size() != 0) {

            double ellipseMajorAxis = std::max(ellipse_pupil.size.width,
                                               ellipse_pupil.size.height);

            roiW = (int)(trackerSettings.ROI_MULTIPLIER * ellipseMajorAxis + 0.5);

        }

        // restrict the width
        roiW = std::max(roiW, trackerSettings.MIN_ROI_W);
        roiW = std::min(roiW, m_rectCrop.width);

        roiH = roiW;
        roiH = std::min(roiH, m_rectCrop.height);

        // starburst start point
        cv::Point2f sbsp = cv::Point2f(-1, -1);

        if(suggestedStartPoint != NULL) {

            sbsp = *suggestedStartPoint;

        }
        else if(ellipse_pupil.center.x > 0 && ellipse_pupil.center.y > 0) {

            sbsp = ellipse_pupil.center;

        }
        else if(lastValidSBPoint.x > 0 && lastValidSBPoint.y > 0) {

            sbsp = lastValidSBPoint;

        }

        // perform starburst and use the results to define the ROI
        if(!this->starburst.process(m_imgGray, m_rectCrop, sbsp)) {

            m_rectRoi.x      = m_rectCrop.x;
            m_rectRoi.y      = m_rectCrop.y;
            m_rectRoi.width  = std::min(trackerSettings.ROI_W_DEFAULT, m_rectCrop.width);
            m_rectRoi.height = std::min(trackerSettings.ROI_W_DEFAULT, m_rectCrop.height);

            starburst.reset();

            return false;

        }


        // store the last point which produced successfull results.
        // TODO: verify that this also works when sbsp = (-1, -1)
        lastValidSBPoint = sbsp;


        double var    = starburst.getPupilVariance();
        double varAvg = sbVarianceAverager.getAverage();

        if(varAvg != 0 && var > 2.5 * varAvg) {

            starburst.reset();

            m_rectRoi.x      = m_rectCrop.x;
            m_rectRoi.y      = m_rectCrop.y;
            m_rectRoi.width  = std::min(trackerSettings.ROI_W_DEFAULT, m_rectCrop.width);
            m_rectRoi.height = std::min(trackerSettings.ROI_W_DEFAULT, m_rectCrop.height);

            sbVarianceAverager.init(-1);

            return false;
        }

        sbVarianceAverager.add(var);


        int roiW2 = roiW >> 1;
        int roiH2 = roiH >> 1;

        cv::Point2f roiCentre = starburst.getROICenter();

        m_rectRoi.x      = roiCentre.x - roiW2;
        m_rectRoi.y      = roiCentre.y - roiH2;
        m_rectRoi.width	 = roiW;
        m_rectRoi.height = roiH;
        fitRectIntoRect(m_rectCrop, m_rectRoi);

        /*
         * http://en.wikipedia.org/wiki/Variance
         * "In probability theory and statistics, the variance is a measure of
         * how far a set of numbers is spread out"
         *
         * TODO: Think about removing this
         */
        if(1.5 * sqrt(starburst.getPupilVariance()) > roiW) {
            starburst.reset();
        }

        return true;

    }


    void PupilTracker::preprocessImage() {

        cv::Mat imgGrayCrop = cv::Mat(m_imgGray, m_rectCrop);

        /*
         * Note: If you remove the blur from here, add it to trackEyeLids().
         */
        //cv::medianBlur(imgGrayCrop, imgGrayCrop, 5);
        cv::blur(imgGrayCrop, imgGrayCrop, cv::Size(3, 3));
        cv::equalizeHist(imgGrayCrop, imgGrayCrop);

    }


    void PupilTracker::checkCropArea() {

        m_rectCrop.x      = trackerSettings.CROP_AREA_X;
        m_rectCrop.y      = trackerSettings.CROP_AREA_Y;
        m_rectCrop.width  = trackerSettings.CROP_AREA_W;
        m_rectCrop.height = trackerSettings.CROP_AREA_H;

        if(m_rectCrop.x < 0      ||
           m_rectCrop.y < 0      ||
           m_rectCrop.width <= 0 ||
           m_rectCrop.height <= 0) {

            m_rectCrop = cv::Rect();

        }

        int imgWidth  = m_imgGray.cols;
        int imgHeight = m_imgGray.rows;


        if(m_rectCrop.width == 0 || m_rectCrop.height == 0 ||
           m_rectCrop.x >= imgWidth || m_rectCrop.y >= imgHeight) {

            m_rectCrop = cv::Rect(0, 0, m_imgGray.cols, m_imgGray.rows);
            return;
        }

        if(m_rectCrop.x + m_rectCrop.width - 1 >= imgWidth) {

            printf("Restricting the width of the cropped area\n"
                   "  old: (%d, %d, %d, %d)\n",
                   m_rectCrop.x,
                   m_rectCrop.y,
                   m_rectCrop.width,
                   m_rectCrop.height);

            m_rectCrop.width = imgWidth - m_rectCrop.x;

            printf("  new: (%d, %d, %d, %d)\n",
                   m_rectCrop.x,
                   m_rectCrop.y,
                   m_rectCrop.width,
                   m_rectCrop.height);

        }

        if(m_rectCrop.y + m_rectCrop.height - 1 >= imgHeight) {

            printf("Restricting the height of the cropped area\n"
                   "  old: (%d, %d, %d, %d)\n",
                   m_rectCrop.x,
                   m_rectCrop.y,
                   m_rectCrop.width,
                   m_rectCrop.height);

            m_rectCrop.height = imgHeight - m_rectCrop.y;

            printf("  new: (%d, %d, %d, %d)\n",
                   m_rectCrop.x,
                   m_rectCrop.y,
                   m_rectCrop.width,
                   m_rectCrop.height);

        }


    }



    /*
     *	This method loops through all clusters and returns the best pupil
     *	candidate's cluster and ellipse indices. Double ellipse fitting is
     *	used in the following way:
     *
     *		1. Get the cluster's centre of mass and search for bounds belonging
     *		   to that cluster and fit an ellipse to the found points.
     *
     *		2. repeat step 1 but now using the ellipse centre
     *
     *		3. Choose the best ellipse in terms of the fitting error
     */
    bool PupilTracker::getPupilFromClusters() {

        double err = HUGE_ERROR_DBL, smallest_err = HUGE_ERROR_DBL;

        /*
         * An ellipse fit with the smallest error does not necessarily mean
         * that this ellipse is best in terms of being similar to the previous
         * results. Therefore, the ellipse with the smallest error will be
         * chosen only, if the best ellipse, with respect to previous frames,
         * was not found. However, the major axis of the ellipse with the smallest
         * error will be stored as a part of a measure of goodness in frames to
         * come. This method was first introduced by Arto Meriläinen in his
         * Master's Thesis.
         */
        const Cluster *cluster_best_coherence = NULL;
        cv::RotatedRect ellipse_best_coherence;

        // these represent the candidate with the smallest fitting error
        const Cluster *cluster_with_smallest_err = NULL;
        cv::RotatedRect ellipse_with_smallest_err;


        // get the clusters from the clusteriser
        const std::vector<Cluster> &clusters		 = clusteriser.getClusters();
        const std::vector<int> &ind_clusters = clusteriser.getClusterIndices();

        // the number of pupil candidates
        unsigned int nCandidates = 0;


        /******************************************************************
         * Get the best pupil candidate from the clusters
         ******************************************************************/

        std::vector<int>::const_iterator it_ind_clusters = ind_clusters.begin();
        std::vector<int>::const_iterator itEndClusters	 = ind_clusters.end();

        clusterLabels.resize(ind_clusters.size(), 0);
        std::vector<int>::iterator itLabel = clusterLabels.begin();

        for(; it_ind_clusters != itEndClusters; ++it_ind_clusters) {

            // get a reference to the current cluster
            const Cluster *curCluster = &(clusters[*it_ind_clusters]);


            /****************************************************************
             * Test the cluster
             ****************************************************************/

            if(!testCluster(*curCluster)) {
                *itLabel = -1;
                ++itLabel;
                continue;
            }


            /****************************************************************
             * Double ellipse fitting
             ****************************************************************/

            // holds the edge points
            std::vector<cv::Point> edgePoints;

            // resulting ellipse
            cv::RotatedRect curEllipse;

            // fit
            if(!doubleEllipseFit(*curCluster, curEllipse, edgePoints)) {
                *itLabel = -2;
                ++itLabel;
                continue;
            }


            /****************************************************************
             * Get the error of the candidate and perform two tests
             * described below.
             ****************************************************************/

            double errCandidate;
            if(!testPupilCandidate(curEllipse, edgePoints.size(), errCandidate)) {
                *itLabel = -3;
                ++itLabel;
                continue;
            }

            // we got this far, so there is a candidate
            ++nCandidates;


            /****************************************************************
             * First evaluation:
             *   Evaluate the candidate's consistency with the previously
             *   selected pupils.
             ****************************************************************/

            if(errCandidate < err) {

                if(isCoherent(curEllipse)) {

                    ellipse_best_coherence	= curEllipse;
                    cluster_best_coherence	= curCluster;
                    err						= errCandidate;

                }

            }


            /****************************************************************
             * Second evaluation:
             *   Evaluate the candidate purely by its error. This value is
             *   inserted into the list that each following frame must agree.
             ****************************************************************/

            if(errCandidate < smallest_err) {

                cluster_with_smallest_err	= curCluster;
                ellipse_with_smallest_err	= curEllipse;
                smallest_err				= errCandidate;

            }

            ++itLabel;

        } // end of for -loop


        if(nCandidates > 0) {

            /*
             * If there are no coherent clusters, select the cluster with
             * the smallest error. This makes the algorithm a bit more
             * adaptive.
             */
            if(cluster_best_coherence == NULL) {

                cluster_pupil = *cluster_with_smallest_err;
                ellipse_pupil =  ellipse_with_smallest_err;

            }
            else {

                cluster_pupil = *cluster_with_smallest_err;
                ellipse_pupil =  ellipse_with_smallest_err;

            }

            // Delete the data of the first verification frame
            if(this->previous_ellipses.size() == trackerSettings.NOF_VERIFICATION_FRAMES) {
                this->previous_ellipses.erase(this->previous_ellipses.begin());
            }

            // Take the major axis of the best fitting ellipse...
            double majorAxis = std::max(ellipse_with_smallest_err.size.height,
                                        ellipse_with_smallest_err.size.width);

            // ...and add it to the list
            this->previous_ellipses.push_back(majorAxis);

            return true;

        }


        // this will be reached only if no candidates were found
        if(this->previous_ellipses.size()) {
            this->previous_ellipses.erase(this->previous_ellipses.begin());
        }

        return false;

    }


    bool PupilTracker::isCoherent(const cv::RotatedRect &ellipse) {

        // Magic number from Kiyama's paper.
        double magicNumber = m_imgGray.cols / 120.0;

        /*
         * Check whether the last N pupils agree with the pupil candidate.
         * If there are more than MAX_NOF_BAD_FRAMES pupil ellipse fits that
         * disagree with the current error, keep the current best pupil.
         */

        unsigned int nBadFrames				= 0;
        std::vector<double>::iterator it	= this->previous_ellipses.begin();
        std::vector<double>::iterator end	= this->previous_ellipses.end();
        double majorAxis					= std::max(ellipse.size.height,
                                                       ellipse.size.width);

        for(; it != end; ++it) {

            if(std::abs(majorAxis - *it) > magicNumber) {

                ++nBadFrames;

            }

        }

        // Did the previous frames agree with the current estimate?
        return nBadFrames <= trackerSettings.MAX_NOF_BAD_FRAMES;

    }


    bool PupilTracker::testCluster(const Cluster &cluster) {

        // consider only clusters containing a number of points within a certain range
        int sz_cluster = (int)cluster.size();

        if(sz_cluster < trackerSettings.MIN_CLUSTER_SIZE ||
           sz_cluster > trackerSettings.MAX_CLUSTER_SIZE) {

            return false;

        }

        // get the start and end points
        int x_start	= cluster.front().x;
        int y_start	= cluster.front().y;
        int x_end	= cluster.back().x;
        int y_end	= cluster.back().y;

        /*
         * See if the start and end points meet and that the area
         * is greater than the minimum defined area.
         */
        if(std::abs(x_start - x_end) > 1 ||
           std::abs(y_start - y_end) > 1 ||
           cv::contourArea(cluster) <= trackerSettings.MIN_PUPIL_AREA) {

            return false;

        }

        return true;

    }


    bool PupilTracker::doubleEllipseFit(const Cluster &curCluster,
                                        cv::RotatedRect &ellipse,
                                        std::vector<cv::Point> &edgePoints) {

        // the number of ellipse fittings
        static const int nEllipseFits = 2;

        // the center of mass of the current cluster
        cv::Point com = getCenterOfMass(curCluster);

        for(int cFit = 0; cFit < nEllipseFits; ++cFit) {

            // clear the list
            edgePoints.clear();

            /*****************************************************************
             * Collect edge points of the cluster starting from the center and
             * moving radially between -45 to 45 degrees towards the edges.
             *****************************************************************/

            getEdgePoints(edgePoints, com.x, com.y);

            // see that enough radiuses were found
            if((int)edgePoints.size() < trackerSettings.MIN_NOF_RADIUSES) {

                edgePoints.clear();

                return false;

            }


            /*******************************************************
             *	The ellipse fitting
             *******************************************************/
            ellipse = cv::fitEllipse(edgePoints);

            /*
             * Sanity check for the ellipse. The boundingrect _must_ be inside the
             * image!
             * TODO: consider limiting the br inside rectRoi
             */
            cv::Rect br = ellipse.boundingRect();
            if((br.x < 0)							||
               (br.x + br.width > m_imgGray.cols)	||
               (br.y < 0)							||
               (br.y + br.height > m_imgGray.rows)) {

                edgePoints.clear();

                return false;

            }

            // update the centre of mass
            com.x = ROUND(ellipse.center.x);
            com.y = ROUND(ellipse.center.y);

        }

        return true;

    }


    void PupilTracker::getEdgePoints(std::vector<cv::Point> &pupil_points, const int xc, const int yc) {

        int sumx = 0;
        int sumy = 0;

        // if the user has changed the number of rays, precompute them again
        if(distX.size() != (size_t)trackerSettings.NOF_RAYS) {
            precomputeRays();
        }


        /* Get the edge points of the cluster by moving radially
           away from the center of mass and compute the mean as well */
        for(int i = 0; i < trackerSettings.NOF_RAYS; ++i) {

            int x2;
            int y2;

            // distX and distY are pre-calculated arrays
            setEndPoints(xc, yc, x2, y2, this->distX[i], this->distY[i]);

            cv::Point pointAtTH;
            cv::Point pointPriorToTH;

            bool success = findFromLine(xc, yc, x2, y2,
                                        BINARY_WHITE,
                                        pointAtTH,
                                        pointPriorToTH);

            // see that the point is valid, i.e. not -1 and that its not the centre of mass
            if(success && (pointPriorToTH.x != xc || pointPriorToTH.y != yc)) {

                pupil_points.push_back(pointPriorToTH);

                sumx += pointPriorToTH.x;
                sumy += pointPriorToTH.y;

            }

        }


        /******************************************************************
         * Remove the outliers
         ******************************************************************/

        double meanx = (double)sumx / (double)pupil_points.size();
        double meany = (double)sumy / (double)pupil_points.size();


        // compute the variance
        cv::Point2f center(0.0, 0.0);
        int square_x = 0, square_y = 0;

        for(std::vector<cv::Point>::iterator point = pupil_points.begin();
            point != pupil_points.end();
            ++point) {

            center.x += point->x;
            center.y += point->y;
            square_x += point->x * point->x;
            square_y += point->y * point->y;

        }

        center.x /= (double)pupil_points.size();
        center.y /= (double)pupil_points.size();

        square_x /= (double)pupil_points.size();
        square_y /= (double)pupil_points.size();

        double var_x = (center.x * center.x - (double)square_x);
        double var_y = (center.y * center.y - (double)square_y);

        double var = std::sqrt(var_x * var_x + var_y * var_y);


        // remove the outliers
        for(std::vector<cv::Point>::iterator point = pupil_points.begin(); point != pupil_points.end(); ++point) {

            double diff_x = point->x - meanx;
            double diff_y = point->y - meany;

            if(sqrt(diff_x * diff_x + diff_y * diff_y) > (sqrt((double)var) * 1.5)) {

                // Take the element to remove
                std::vector<cv::Point>::iterator remove_pos = point;

                // Is this the first element?
                if(remove_pos != pupil_points.begin()) {
                    // No.. just remove it


                    --point;
                    pupil_points.erase(remove_pos);
                } else {
                    // We're removing the first element... remove and reset the iterator.
                    pupil_points.erase(remove_pos);
                    point = pupil_points.begin();
                }
            }
        }

    }


    /* n = the number of edge points used in fitting the ellipse */
    bool PupilTracker::testPupilCandidate(const cv::RotatedRect &e, int n, double &err) {

        err = HUGE_ERROR_DBL;

        //        if(e.size.width < 5.0 || e.size.height < 5.0) {
        if(e.size.width < 2*trackerSettings.MIN_PUPIL_RADIUS || e.size.height < 2*trackerSettings.MIN_PUPIL_RADIUS) {
            //            printf("PupilTracker::testPupilCandidate(): ellipse too small\n");
            return false;
        }

        /**********************************************************************
         * Define the size for the area that is used in comparison (a kind
         * of local ROI)
         *********************************************************************/

        // bounding rectangle
        cv::Rect br = e.boundingRect();

        // Make sure that the ellipse is inside the ROI
        if(br.x < m_rectRoi.x									||
           br.x + br.width - 1 >= (m_rectRoi.x + m_rectRoi.width)	||
           br.y < m_rectRoi.y									||
           br.y + br.height - 1 >= (m_rectRoi.y + m_rectRoi.width)) {

            return false;

        }


        // Get an image that has only the ellipse
        cv::Mat roi_image(m_imgGray, br);

        cv::Mat img_ellipse = roi_image.clone();

        // Create an template image that should resemble the ellipse
        cv::Mat img_template = cv::Mat(img_ellipse.size(), CV_8UC1);

        // Create an ellipse inside the template image
        cv::rectangle(img_template,
                      cv::Point(0, 0),
                      cv::Point(img_template.cols-1, img_template.rows-1),
                      cv::Scalar(BINARY_WHITE),
                      CV_FILLED);

        cv::ellipse(img_template, e, cv::Scalar(BINARY_BLACK), CV_FILLED, CV_AA);

        // Subtract the images.
        img_ellipse.convertTo(img_ellipse, img_ellipse.type(), 0.5, 128);
        img_template.convertTo(img_template, img_template.type(), 0.5, 0);
        cv::Mat result_image = img_ellipse - img_template;

        unsigned long sum = 0;
        int pixels = result_image.rows * result_image.cols;
        unsigned char *data = result_image.data;
        for(int i = 0; i < pixels; ++i) {
            sum += data[i];
        }

        err = ((double)sum / (double)pixels) * ((double)trackerSettings.NOF_RAYS / (double)n);

        return true;

    }


    cv::Point computeMassCentre(const std::vector<cv::Point> &vec) {

        size_t n = vec.size();
        unsigned long sumx = 0;
        unsigned long sumy = 0;
        for(size_t i = 0; i < n; ++i) {
            sumx += vec[i].x;
            sumy += vec[i].y;
        }

        cv::Point ret;
        ret.x = (int)(sumx / (double)n + 0.5);
        ret.y = (int)(sumy / (double)n + 0.5);

        return ret;

    }


    bool fnctSortStdList(ERR err1, ERR err2) {
        return (err1.err < err2.err);
    }


    cv::RotatedRect PupilTracker::trackEyeLids() {

        // define a roi based on the pupil size
        const cv::Rect bb = ellipse_pupil.boundingRect();
        const int nMult = 6;
        const int nNewW = nMult * bb.width;
        const int nNewH = nMult * bb.height;

        int xs = ellipse_pupil.center.x - (nNewW >> 1);
        int xe = xs + nNewW - 1;
        int ys = ellipse_pupil.center.y - (nNewH >> 1);
        int ye = ys + nNewH - 1;
        cv::Rect roi(xs, ys, xe - xs + 1, ye - ys + 1);

        // restrict the roi to be within m_rectCrop
        fitRectIntoRect(m_rectCrop, roi);
        cv::Mat imgGrayCrop = cv::Mat(m_imgGray, roi).clone();

        /*
         * NOTE: Do not blur here, aready done in preprocessImage()
         */
        // blur to get the iris smoother
        //cv::blur(imgGrayCrop, imgGrayCrop, cv::Size(5, 5), cv::Point(-1, -1));
        //cv::medianBlur(imgGrayCrop, imgGrayCrop, 27);

        /**********************************************************************
         * Burst
         **********************************************************************/
        double c = 1.4;
        const cv::RotatedRect *pEll = getEllipsePupil();
        cv::Size sz(c * pEll->size.width,
                    c * pEll->size.height);

        const cv::RotatedRect ell(pEll->center, sz, pEll->angle);

        std::vector<cv::Point2f> vecEdges;
        bool bBurstOk = iris::burst(m_imgGray, roi, ell, vecEdges);

        if(!bBurstOk) {

            const double ellipseMajorAxis = std::max(ellipse_pupil.size.width,
                                                     ellipse_pupil.size.height);

            float fRoiW = 3.5 * ellipseMajorAxis;

            // search ellipse
            return cv::RotatedRect(ellipse_pupil.center,
                                   cv::Size2f(fRoiW, fRoiW),
                                   ellipse_pupil.angle);

        }


        /*********************************************************
         * Select the top and bottom samples from the rays.
         *********************************************************/
        std::vector<cv::Point2f> vecTB; // top-bottom
        for(size_t i = 0; i < vecEdges.size(); ++i) {

            const cv::Point2f &curPoint = vecEdges[i];
            const double x = curPoint.x - pEll->center.x;
            const double y = curPoint.y - pEll->center.y;

            const double dAng = atan2(y, x);
            const double dAngAbs = fabs(dAng);

            double dLimit = pi*0.25;

            if(dAngAbs > dLimit && (pi - dAngAbs) > dLimit) { // 0.25 * pi = pi / 4
                vecTB.push_back(curPoint);
            }

        }

        /*************************************************
         * Fit ellipses to both point sets.
         * ellipse fitting requires at least five points.
         *************************************************/
        cv::RotatedRect ellTB = vecTB.size() >= 5 ? cv::fitEllipse(vecTB) : cv::RotatedRect();
        ellTB.size.height *= 0.9;
        ellTB.size.width  *= 0.9;

        return ellTB;

    }


    void fitRectIntoRect(const cv::Rect &rectBig, cv::Rect &rectSmall) {

        // inclusive
        int xeMax = rectBig.x + rectBig.width  - 1;
        int yeMax = rectBig.y + rectBig.height - 1;


        // start point
        int &xs = rectSmall.x;
        int &ys = rectSmall.y;

        // sanity checks
        if(xs < rectBig.x || xs >= xeMax) {
            xs = rectBig.x;
        }
        if(ys < rectBig.y || ys >= yeMax) {
            ys = rectBig.y;
        }

        // sanity checks
        int &rW = rectSmall.width;
        int &rH = rectSmall.height;
        rW = xs + rW - 1 <= xeMax ? rW : xeMax - xs + 1;
        rH = ys + rH - 1 <= yeMax ? rH : yeMax - ys + 1;

    }


    /*
     * An estimate for the pupil has to have been found, before a call to
     * this function.
     */
    void PupilTracker::getCrCandidates(std::list<ERR> &listCrCandidates) {

        cv::Rect crRoi = m_crSearchEllipse.boundingRect();
        fitRectIntoRect(m_rectRoi, crRoi);

        // copy the part of the gray-scale image where CRs are looked for
        const cv::Mat imgGrayCR = cv::Mat(m_imgGray, crRoi);//.clone(); // deep copy


        // threshold the sub-image
        cv::Mat imgBinaryCrs;
        cv::threshold(imgGrayCR, imgBinaryCrs, thresholds.cr, 255, cv::THRESH_BINARY);

        /************************************************************
         * Now find the contours and test them with a mask
         ************************************************************/

        std::vector<Cluster> crContours;

        cv::findContours(imgBinaryCrs,
                         crContours,
                         CV_RETR_LIST,
                         CV_CHAIN_APPROX_NONE,
                         cv::Point(crRoi.x, crRoi.y));

        // create a new CR template image
        cv::Mat imgCrTemplate;
        CRTemplate::makeTemplateImage(imgCrTemplate,
                                      trackerSettings.CR_MASK_LEN,          // width and height
                                      trackerSettings.MAX_CR_WIDTH * 0.5);  // cr radius

        const int nContours = (int)crContours.size();
        for(int i = 0; i < nContours; ++i) {

            // mass centre
            const cv::Point2f com = PupilTracker::computeComOfCluster(crContours[i]);

            if(!ellipse::pointInsideEllipse(m_crSearchEllipse, com)) {
                continue;
            }


            ERR error;
            error.point = cv::Point((int)(com.x + 0.5),
                                    (int)(com.y + 0.5));

            // test with a mask
            error.err = CRTemplate::maskTests(imgGrayCR,
                                              error.point - cv::Point(crRoi.x, crRoi.y),
                                              imgCrTemplate);

            // test circularity
            // error.err += CRTemplate::testCircularity(imgBinaryCrs,
            //                                          error.point - cv::Point(xs, ys));

            listCrCandidates.push_back(error);

        }

    }


    // static function
    cv::Point2f PupilTracker::computeComOfCluster(const Cluster &c) {

        const int n = (int)c.size();

        unsigned int x = 0;
        unsigned int y = 0;

        for(int i = 0; i < n; ++i) {

            x += c[i].x;
            y += c[i].y;

        }

        return cv::Point2f((double)x / (double)n,
                           (double)y / (double)n);

    }


    void PupilTracker::selectBestCrCandidates(std::list<ERR> &listCrCandidates) {

        // sort so that the smallest errors, i.e. the best candidates, are first
        listCrCandidates.sort(fnctSortStdList);

        // get the error of the best candidate
        const double errOfBest = listCrCandidates.begin()->err;

        /*
         * Compute the maximum acceptable error of a candidate.
         * CR_MAX_ERR_MULTIPLIER is defined between [0..1].
         *
         *   CR_MAX_ERR_MULTIPLIER *  curErr < errOfBest
         *   => maxErr = errOfBest / CR_MAX_ERR_MULTIPLIER
         */
        const double maxAcceptableError = errOfBest / trackerSettings.CR_MAX_ERR_MULTIPLIER;

        // define a minimum distance between the CRs
        const unsigned int minAcceptableDist = 1.5*trackerSettings.MAX_CR_WIDTH;

        // how many crs have been flooded
        int cFlooded = 0;

        int max_nof_crs = crs.getMax();
        std::vector<cv::Point2d> &cr_centres = crs.getCentres();
        cr_centres.clear();


        const int w = m_imgGray.cols;
        const int h = m_imgGray.rows;
        const int step = m_imgGray.step;
        const unsigned char *data = m_imgGray.data;

        int half_len = trackerSettings.MAX_CR_WIDTH / 2;

        // floodfill the best candidates and compute their mass centres

        std::list<ERR>::iterator itCandidate = listCrCandidates.begin();
        std::list<ERR>::iterator endCandidate = listCrCandidates.end();

        for( ; cFlooded < max_nof_crs && itCandidate != endCandidate; ++itCandidate) {

            const cv::Point &curPoint = itCandidate->point;

            int x = curPoint.x;
            int y = curPoint.y;

            if(x - half_len >= 0 &&
               y - half_len >= 0 &&
               x + half_len < w  &&
               y + half_len < h) {

                // error of the current candidate
                const double curErr = itCandidate->err;

                // discard candidates whose errors are much worse than the best candidate's
                if(curErr > maxAcceptableError) {

                    /*
                     * The reason to break the entire loop here is
                     * because the list has been sorted in ascending
                     * order according to the candidate errors. So if
                     * the current candidate's error exceeds the maximum
                     * acceptable error, it is guaranteed that the remaining
                     * candidates' errors are too large as well.
                     */
                    break;

                }


                // compute the minimun distance to the previous CRs
                unsigned int minDist = HUGE_ERROR_UINT;

                for(int c = 0; c < cFlooded; ++c) {

                    double diff_x = x - cr_centres[c].x;
                    double diff_y = y - cr_centres[c].y;
                    unsigned int curDist = (unsigned int)(std::sqrt(diff_x * diff_x + diff_y * diff_y) + 0.5);

                    // store the minimum distance
                    if(curDist < minDist) {

                        minDist = curDist;

                        // check that the distance is not too short
                        if(minDist < minAcceptableDist) {

                            break;

                        }

                    }

                }

                int index = y*step + x;

                // might have been purposely set to zero in the previous flood fill
                if(minDist >= minAcceptableDist/* && data[index] > 0*/) {

                    unsigned char th = (unsigned char)(TH_FF * data[index] + 0.5);
                    cv::Rect rect_com(x - half_len, y - half_len, trackerSettings.MAX_CR_WIDTH, trackerSettings.MAX_CR_WIDTH);
                    cv::Point2d tmp;
                    this->getCOM_nonrecursive(cv::Point(x, y), rect_com, tmp, th);
                    cr_centres.push_back(tmp);
                    ++cFlooded;

                }

            }

        } // end of for -loop


        // sort CRs
        std::vector<cv::Point2d> &centres = crs.getCentres();
        std::sort(centres.begin(), centres.end(), sort_crs);

    }


    bool PupilTracker::findCornealReflections() {

        /*****************************************************************
         * First populate the list of candidates. List of CR candidates,
         * consist points that are centres for birght areas.
         *****************************************************************/

        std::list<ERR> listCrCandidates;

        getCrCandidates(listCrCandidates);

        if(listCrCandidates.size() == 0) {
            return false;
        }
 

        /*****************************************************************
         * Now select the best candidates as the CRs
         *****************************************************************/

        selectBestCrCandidates(listCrCandidates);


        return true;

    }


    cv::Point PupilTracker::getCenterOfMass(const std::vector<cv::Point> &cluster) {

        int nOfPoints = (int)cluster.size();

        unsigned int sumX = 0;
        unsigned int sumY = 0;
        for(int i = 0; i < nOfPoints; ++i) {
            sumX += cluster[i].x;
            sumY += cluster[i].y;
        }

        int cx = (int)((double)sumX / nOfPoints + 0.5);
        int cy = (int)((double)sumY / nOfPoints + 0.5);

        cv::Point ret(cx, cy);

        return ret;
    }


    /*
     * Looks for the value defined by threshold from the binary image.
     * x1 and y1 define the starting point and x2 any y2 define the
     * end point. These points must be within the image. The function
     * returns the point prior to the thresohold value and the point
     * at the threhold value. Returns true or false.
     */
    bool PupilTracker::findFromLine(int x0,
                                    int y0,
                                    int x1,
                                    int y1,
                                    unsigned char threshold,
                                    cv::Point &pointAtTH,
                                    cv::Point &pointPriorToTH) {

        /*
         * http://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
         *
         * This is based on an optimised integer based line drawing method.
         * Instead of drawing the line we go along it and detect a bright pixel.
         */
        bool steep = abs(y1 - y0) > abs(x1 - x0);

        pointAtTH.x	= x0;
        pointAtTH.y	= y0;
        pointPriorToTH.x		= x0;
        pointPriorToTH.y		= y0;

        if(steep) {
            std::swap(x0, y0);
            std::swap(x1, y1);
        }

        int deltax = abs(x1 - x0);
        int deltay = abs(y1 - y0);
        int error = deltax / 2;
        int x = x0;
        int y = y0;
        int ystep = y0 < y1 ? 1 : -1;

        int inc = x0 < x1 ? 1 : -1;

        int *px = &x;
        int *py = &y;

        if(steep) {
            px = &y;
            py = &x;
        }


        const unsigned char * const pixels = m_imgBinary.data;
        const int step = m_imgBinary.step;

        while(x != x1) {

            if(pixels[step * (*py) + (*px)] == threshold) {

                pointAtTH.x = *px;
                pointAtTH.y = *py;

                return true;

            }

            pointPriorToTH.x = *px;
            pointPriorToTH.y = *py;

            error -= deltay;
            if(error < 0) {
                y += ystep;
                error += deltax;
            }

            x += inc;

        }

        return false;

    }


    void PupilTracker::setEndPoints(const int x1,
                                    const int y1,
                                    int &x2,
                                    int &y2,
                                    int distX,
                                    int distY) {

        // inclusive bounds
        const int startX = m_rectCrop.x;
        const int startY = m_rectCrop.y;
        const int endX   = m_rectCrop.x + m_rectCrop.width - 1;
        const int endY   = m_rectCrop.y + m_rectCrop.height - 1;


        // end points of the ray. Give a value for x2 and y2
        x2 = x1 + distX;
        y2 = y1 + distY;

        /*
         * If x2 is not within the acceptable bounds, restrict its value. Set y2
         * according to x2.
         */
        if(x2 < startX) {

            int x2_old = x2;
            x2 = startX;

            /*
             * Solve for y2
             *
             *    (y2 - y1)   (y2' - y1)
             *    ---------	= -----------
             *    (x2 - x1)   (x2' - x1)
             *
             * In this case (x2' - x1') != 0 (actually < 0), because
             * x2' < 0, but x1', which is always valid is always x1' >= 0
             *
             */
            y2 = (int)((y2 - y1) * (x2 - x1) / (double)(x2_old - x1) + y1 + 0.5);

        }
        else if(x2 > endX) {
            int x2_old = x2;
            x2 = endX;
            y2 = (int)((y2 - y1) * (x2 - x1) / (double)(x2_old - x1) + y1 + 0.5);
        }

        /*
         * If y2 is not within the acceptable bounds, restrict its value and set
         * x2 accordingly. Above x2 was already set and possibly restricted. Will
         * setting it according to y2 possibly put it out of the acceptable range?.
         * The answer is no, because here x2 is adjusted in such a way that it gets
         * closer to x1.
         */
        if(y2 < startY) {
            int y2_old = y2;
            y2 = startY;
            x2 = (int)((x2 - x1) * (y2 - y1) / (double)(y2_old - y1) + x1 + 0.5);
        }
        else if(y2 > endY) {
            int y2_old = y2;
            y2 = endY;
            x2 = (int)((x2 - x1) * (y2 - y1) / (double)(y2_old - y1) + x1 + 0.5);
        }

    }


    // http://stackoverflow.com/questions/1257117/does-anyone-have-a-working-non-recursive-floodfill-algorithm-written-in-c
    void PupilTracker::getCOM_nonrecursive(const cv::Point &point,
                                           const cv::Rect &rectROI,
                                           cv::Point2d &com,
                                           unsigned char th) {

        uint64_t cumul_x	= 0;
        uint64_t cumul_y	= 0;
        uint64_t count		= 0;

        const cv::Mat imgGrayROI	= cv::Mat(m_imgGray, rectROI);
        cv::Mat imgGrayROICopy		= imgGrayROI.clone();
        unsigned char *dataGrayCopy	= imgGrayROICopy.data;
        int step					= imgGrayROICopy.step;

        const int widthROI	= rectROI.width;
        const int heightROI	= rectROI.height;
        const int areaROI	= widthROI * heightROI;


        // map the point to the ROI
        std::list<int> stack;
        stack.push_back(
                        (point.x - rectROI.x) +			// x
                        (point.y - rectROI.y) * step	// y
                        );

        while(stack.size() > 0) {

            int p = stack.back();
            stack.pop_back();

            if(p < 0 || p >= areaROI) {
                continue;	// move back to "while(stack.size() > 0)"
            }

            unsigned char val = dataGrayCopy[p];
            if(val > th) {

                int x = p % step;
                int y = p / step;


                /*
                 * Paint the grayscale pixel black so that the another possible
                 * check above does not result in coming here again
                 */
                dataGrayCopy[p]		= 0;

                cumul_x				+= x;
                cumul_y				+= y;
                ++count;

                stack.push_back(p + 1);		// right
                stack.push_back(p - 1);		// left
                stack.push_back(p + step);	// below
                stack.push_back(p - step);	// above

            }

        }

        if(count > 0) {
            com.x = ((double)cumul_x / (double)count) + rectROI.x;
            com.y = ((double)cumul_y / (double)count) + rectROI.y;
        }
        else {
            com = point;
        }

    }


    void PupilTracker::clearVars() {

        this->ellipse_pupil = cv::RotatedRect();

        this->cluster_pupil.clear();

        crs.getCentres().clear();

    }




    /*****************************************************
     * Threshold averaging class starts from here
     *****************************************************/

    ThresholdAverager::ThresholdAverager() {

        sum		= 0;
        average	= 0;
        count	= 0;
        index	= 0;

    }


    void ThresholdAverager::init(int _buffSz) {

        // initialise with zeros
        values.resize(_buffSz, 0);

    }


    void ThresholdAverager::add(unsigned char val) {

        unsigned int sz = values.size();

        if(count < sz) {
            ++count;
        }

        // remove the oldest value
        sum -= values[index];

        // add the value to the sum...
        sum += val;

        // ...and to the list
        values[index] = val;

        // compute the average
        average = (unsigned char)((double)sum / (double)count + 0.5);

        // compute the index
        index = (index + 1) % sz;

    }



    /*****************************************************
     * Variance averaging class starts from here
     *****************************************************/

    VarianceAverager::VarianceAverager() {

        sum		= 0;
        average	= 0;
        count	= 0;
        index	= 0;

    }


    void VarianceAverager::init(int _buffSz) {

        if(_buffSz == -1) {

            // initialise with zeros
            values.assign(values.size(), 0);

        }
        else {
            // initialise with zeros
            values.resize(_buffSz, 0);
        }

        sum = 0;
        average = 0;
        count = 0;
        index = 0;

    }


    void VarianceAverager::add(double val) {

        unsigned int sz = values.size();

        if(count < sz) {
            ++count;
        }

        // remove the oldest value
        sum -= values[index];

        // add the value to the sum...
        sum += val;

        // ...and to the list
        values[index] = val;

        // compute the average
        average = sum / (double)count;

        // compute the index
        index = (index + 1) % sz;

    }



}	// end of namespace gt {

