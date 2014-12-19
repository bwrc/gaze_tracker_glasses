#ifndef PUPILTRACKER_H
#define PUPILTRACKER_H

#include <string.h>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "starburst.h"
#include "clusteriser.h"
#include "CRTemplate.h"

#define AREA(X) (3.14159265 * (X) * (X))

/* black becomes 255 because of inverse thresholding */
static const unsigned char BINARY_BLACK = 255;

/* white becomes 0 because of inverse thresholding */
static const unsigned BINARY_WHITE = 0;


/* Use value from N-1 previous frames for determining the threshold */
extern int SAMPLES_TO_AVERAGE;


namespace gt {


    /*
     * This class computes the average of the last buffSz values.
     * As a new value is given, the oldest value is dropped, if
     * the buffer is full, and the average is computed.
     */
    class ThresholdAverager {

	public:

		ThresholdAverager();

		/* initialise the instance with the given buffer size. */
		void init(int _buffSz);

		/* Add a new value and compute the average. */
		void add(unsigned char val);

		/* Return the average value. */
		unsigned char getAverage() {return average;}

	private:

		unsigned int sum;		// sum of the values
		unsigned char average;	// average value
		unsigned int count;		// how many samples have been added. Count until values.size() is reached.

		std::vector<unsigned char> values;	// up to buffSz values
		unsigned int index;					// index to which place the next value

    };




    /*
     * This class computes the average of the last buffSz values.
     * As a new value is given, the oldest value is dropped, if
     * the buffer is full, and the average is computed.
     */
    class VarianceAverager {

	public:

		VarianceAverager();

		/* initialise the instance with the given buffer size. */
		void init(int _buffSz);

		/* Add a new value and compute the average. */
		void add(double val);

		/* Return the average value. */
		double getAverage() {return average;}

	private:

		double sum;                 // sum of the values
		double average;             // average value
		unsigned int count;         // how many samples have been added. Count until values.size() is reached.

		std::vector<double> values;	// up to buffSz values
		unsigned int index;			// index to which place the next value

    };


    /* A container for the thresholds */
    class Thresholds {

	public:

		Thresholds() {
			pupil	= 0;
			cr		= 0;
		}

		int pupil;
		int cr;

    };


    struct ERR {
        cv::Point point;
        double err;
    };


    class PupilTracker {

	public:

		PupilTracker();
		~PupilTracker();

        /*
         * Reset the pupil tracker. The next call to track() will
         * not use previous tracking results as refernces. This function
         * is also called in track() right after having used previous
         * results.
         */
		void reset();

		/*
		 * Track the pupil and the glints. The input image will be
		 * copied. All operations will be applied to the copy image.
		 */
		bool track(const cv::Mat &_imgGray, const cv::Point2f *suggestedStartPoint = NULL);


		/*****************************************************************
		 * Accessors
		 *****************************************************************/

		const std::vector<std::vector<cv::Point> > &getClusters() const {
			return clusteriser.getClusters();
		}

		const Clusteriser &getClusteriser() const {
			return clusteriser;
		}

		const std::vector<cv::Point2d> &getCornealReflections() const {return crs.getCentres();}

		const std::vector<cv::Point> &getClusterPupil() const {
			return this->cluster_pupil;
		}

		const cv::Mat &getBinaryImage() {return m_imgBinary;}

		const cv::Mat &getGrayImage() const {return m_imgGray;}

		const cv::Rect &getROI() {return m_rectRoi;}

		Thresholds *getThresholds() {return &this->thresholds;}

		const cv::RotatedRect *getEllipsePupil() const {return &this->ellipse_pupil;}

		const Starburst &getStarburstObject() {return this->starburst;}

        cv::Rect getCropArea() {return m_rectCrop;}

        const std::vector<int> &getClusterLabels() {return clusterLabels;}


		/*****************************************************************
		 * Set parameters
		 *****************************************************************/

        void setCropArea(const cv::Rect &rectCrop) {
            m_rectCrop = rectCrop;
        }

		/* Set the number of glints to detect */
		void set_nof_crs(int n) {
			crs.setMax(n);
		}

		/* Increase the number of glints to detect */
		void inc_nof_crs(int _inc) {
			int suggested = crs.getMax() + _inc;

			if(suggested >= 1 && suggested <= 20) {
				crs.setMax(suggested);
			}
		}

		void useAutoTh(bool t);


        cv::RotatedRect getSearchEllipse() {return m_crSearchEllipse;}

	private:

        /*
         * Forbid the use of a copy constructor and the assignment operator.
         */
        PupilTracker(const PupilTracker &other);
        PupilTracker &operator=(const PupilTracker &other);

        /*
         * Perform preprocessing, like histogram equalisation etc. Performed
         * only for the cropped area.
         */
        void preprocessImage();

		/*
		 * Performs the double ellipse fitting described in:
		 *   "FreeGaze: A Gaze Tracking System for Everyday Gaze Interaction".
		 */
		bool doubleEllipseFit(const std::vector<cv::Point> &curCluster,
							  cv::RotatedRect &ellipse,
							  std::vector<cv::Point> &edgePoints);

		/*
		 * Test if the given ellipse is coherent with the previously
		 * selected ellipses
		 */
		bool isCoherent(const cv::RotatedRect &ellipse);

        /*
         * Collect edge points of the cluster starting from the center and
         * moving radially between -45 to 45 degrees towards the edges.
         * Uses the binary image.
         */
		void getEdgePoints(std::vector<cv::Point> &pupil_points, const int xc, const int yc);

		/* n = the number of edge points used in fitting the ellipse */
		bool testPupilCandidate(const cv::RotatedRect &e, int n, double &err);


		/*
		 * Threshold the image within the ROI with the given threshold
		 * value. The resulting threshold image is stored in img_binary.
		 * The thresholding method inverts colors, i.e. black => white
		 * and white => black.
		 */
		void thresholdImage(int threshold, const cv::Rect &_rectRoi) {

			// if the sizes differ, reallocate img_binary
			if(m_imgGray.size() != m_imgBinary.size()) {
				m_imgBinary.release();
				m_imgBinary = cv::Mat(m_imgGray.size(), CV_8UC1);
			}

			cv::Mat imgGrayRoi   = cv::Mat(m_imgGray, _rectRoi);
			cv::Mat imgBinaryRoi = cv::Mat(m_imgBinary, _rectRoi);

			cv::threshold(imgGrayRoi, imgBinaryRoi, threshold, 255, cv::THRESH_BINARY_INV);
		}


		/*
		 * Define the region of interest based upon the findings from
		 * running starburst. The starburst algorithm estimates the
		 * centre of the roi and how far spread the samples are. The previous
		 * pupil ellipse is used for defining the width of the ROI.
		 */
		bool define_ROI(const cv::Point2f *suggestedStartPoint);

		/*
		 * Get the best pupil candidate from the clusters.
		 * An ellipse fit with the smallest error does not necessarily mean
		 * that this ellipse is best in terms of being similar to the previous
		 * results. Therefore, the ellipse with the smallest error will be
		 * chosen only, if the best ellipse, with respect to previous frames,
		 * was not found. However, the major axis of the ellipse with the
		 * smallest error will be stored as a part of a measure of goodness in
		 * frames to come.
		 * This method was first introduced by Arto Meriläinen in his Master's
		 * Thesis.
		 */
		bool getPupilFromClusters();

        /*
         * Track the eye lids. This function fits and ellipse to the points
         * of the top and bottom eye lids. The obtained ellipse, m_crSearchEllipse
         * will serve as a search area when finding the corneal reflections
         */
        cv::RotatedRect trackEyeLids();

        /*
         * Try to find all corneal reflections inside the ellipse.
         * If the ellipse is not defined, there will be a default search
         * ellipse based on the pupil size.
         *
         * Returns true if at least one CR was found and false,
         * otherwise.
         */
		bool findCornealReflections();

        /*
         * Populate the given list with good CR candidates.
         * An estimate for the pupil has to have been found, before a call to this
         * function.
         */
        void getCrCandidates(std::list<ERR> &listCrCandidates);

        /*
         * Select the best candidates out of the given list. The list
         * must _not_ be empty, so the caller must check that the size
         * is non-zero, when calling this function. This function
         * populates the list of CRs owned by CRs crs. Not maybe the
         * best solution, but easier than returning a list of indices
         * of the chosen candidates or some such method.
         */
        void selectBestCrCandidates(std::list<ERR> &listCrCandidates);

		/*
		 * Test if this cluster is suitable as a candidate for the
		 * pupil.
		 */
		bool testCluster(const std::vector<cv::Point> &cluster);


        /* Check that the crop area is ok, if not adjust. */
        void checkCropArea();

		/*
		 * Computes values for x2 and y2 given the starting point and
		 * distances along the axes. This function restricts the end points
		 * to remain within the image preserving the initial slope of the
		 * desired line.
		 */
		void setEndPoints(const int x1,
						  const int y1,
						  int &x2,
						  int &y2,
						  int distX,
						  int distY);

		/*
		 * Looks for the value defined by threshold from the binary image.
		 * x1 and y1 define the starting point and x2 any y2 define the
		 * end point. These points must be within the image. The function
		 * returns the point prior to the thresohold value and the point
		 * at the threhold value. Returns true or false.
		 */
		bool findFromLine(int x1,
						  int y1,
						  int x2,
						  int y2,
						  unsigned char threshold,
						  cv::Point &ret_bright,
						  cv::Point &ret_dark);

		cv::Point getCenterOfMass(const std::vector<cv::Point> &cluster);

		/*
		 * The start index must be located within the ROI
		 */
		void getCOM_nonrecursive(const cv::Point &point, const cv::Rect &rectROI, cv::Point2d &com, unsigned char th);

		void clearVars();

		// the clusteriser
		Clusteriser clusteriser;


        static cv::Point2f computeComOfCluster(const Cluster &c);


		void precomputeRays();

		std::vector<int> distX;
		std::vector<int> distY;

		/*
		 * This binary image is the result of an inverse-threshold operation.
		 * This means that the dark areas become white and the bright areas become black.
		 */
		cv::Mat m_imgBinary;

		cv::Mat m_imgGray;

        /* The region of interest, ROI */
		cv::Rect m_rectRoi;

        /* The cropped area */
        cv::Rect m_rectCrop;



		/*************************************************************
		 * Results
		 *************************************************************/

		/* Points of the corneal reflections */
		CRs crs;

		cv::RotatedRect ellipse_pupil;
		std::vector<cv::Point> cluster_pupil;

		// clusters should also be here


		/* This variable indicates if the corneal reflections are being tracked */
		bool bTrackCRs;


		Thresholds thresholds;
		ThresholdAverager thresholdAverager;


        /* An averager for the computed variance in starburst */
        VarianceAverager  sbVarianceAverager;

		/*
		 * A vector of the major axis of the pupil ellipses from a predefined
		 * number of previous frames. Used in determining coherence, see
		 * getPupilFromClusters() for more details.
		 */
		std::vector<double> previous_ellipses;

		Starburst starburst;

		int failed_tracks;

        /* The last valid starburst initial point */
        cv::Point2f lastValidSBPoint;

        /* Sotres labels for the pupil cluster candidates, nice for debugging purposes. */
        std::vector<int> clusterLabels;

        /*
         * This ellipse is used as a search area in finding the CRs.
         * The ellipse is extracted in trackEyeLids().
         */
        cv::RotatedRect m_crSearchEllipse;

    };

} // end of namespace gt {


#endif

