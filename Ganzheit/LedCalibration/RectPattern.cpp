#include "RectPattern.h"
#include "imgproc.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>



class ERR {
	public:
		ERR() {
			err = 0.0;
			index = 0;
		}

		ERR(double _err, int _index) {
			err = _err;
			index = _index;
		}

	double err;
	int index;
};


struct myclass {
  bool operator() (ERR i, ERR j) { return (i.err < j.err);}
} myobject;

class VOTES {
	public:
		VOTES() {
			index = 0;
			n_votes = 0;
		}

		VOTES(int _index, int _n_votes) {
			index = _index;
			n_votes = _n_votes;
		}

		int n_votes;
		int index;
};

struct sort_votes {
  bool operator() (VOTES i, VOTES j) { return(i.n_votes > j.n_votes);}
} sort_votes;

class Ellipse {
	public:
		Ellipse() {
			area = 0;
		}
		cv::RotatedRect r;
		double area;
		int n_rays;			// formed using this many rays
};



void organise_corners(std::vector<cv::Point2f> &corners);


RectPattern::RectPattern(double _width) {

	/************************************************************
	 * OpenCV-ccordinate system:
	 *
	 *   x+ : right
	 *   x- : left
	 *   y+ : down
	 *   y- : up
	 *   z+ : towards the monitor
	 *   z- : towards the camera
	 ************************************************************/

	double half = _width / 2.0;

	object_points.resize(4);
	object_points[0] = cv::Point3f(-half, -half, 0.0);	// left up
	object_points[1] = cv::Point3f(-half,  half, 0.0);	// left down
	object_points[2] = cv::Point3f( half,  half, 0.0);	// right down
	object_points[3] = cv::Point3f( half, -half, 0.0);	// right up
}


static const int N_ELLIPSE_RAYS = 100;


bool RectPattern::findMarkers(const cv::Mat &img_gray) {

	image_points.clear();

	std::vector<Ellipse> ellipses;

	cv::Mat img_binary;
	cv::threshold(img_gray, img_binary, 90, 255, cv::THRESH_BINARY_INV);

	std::vector<std::vector<cv::Point> > contours;
	cv::findContours(img_binary, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	for(size_t i = 0; i < contours.size(); ++i) {

		const std::vector<cv::Point> &cur_contour = contours[i];

		if(cur_contour.size() < 20) {
			continue;
		}

		if(std::abs(cur_contour.back().x - cur_contour.front().x) > 1 ||
		   std::abs(cur_contour.back().y - cur_contour.front().y) > 1) {
			continue;
		}

		int radius = img_gray.rows / 4.0;

		cv::Point2f com;
		imgproc::getCOM(cur_contour, com);


		double pi = 3.14159265;
		double ang_d = (2.0*pi) / N_ELLIPSE_RAYS;
		double ang = 0;

		std::vector<cv::Point> edges;

		for(int n_ang = 0; n_ang < N_ELLIPSE_RAYS; ++n_ang) {

			int x0 = com.x;
			int y0 = com.y;
			int x1;
			int y1;

			imgproc::setEndPoints(x0,								// start x
								  y0,								// start y
								  x1,								// result x
								  y1,								// result y
								  (int)round(radius * cos(ang)),	// given distance x
								  (int)round(radius * sin(ang)),	// given distance y
								  img_gray.cols,					// image width
								  img_gray.rows);					// image height
			ang += ang_d;

			cv::Point ret_bright, ret_dark;

			imgproc::findFromLine(img_gray,
								  x0,
								  y0,
								  x1,
								  y1,
								  90,
								  ret_bright,
								  ret_dark);

			if(ret_dark.x != -1) {
				edges.push_back(ret_dark);
			}

		}


		// now we have the edge points. Next we remove the outliers
		imgproc::removeOutliers(edges, 1.5);


		// at least 80 % of the rays should be found
		if(edges.size() >= 0.80*N_ELLIPSE_RAYS) {

			Ellipse ellipse;
			ellipse.r		= cv::fitEllipse(cv::Mat(edges));
			double small	= std::min(ellipse.r.size.width, ellipse.r.size.height);
			double big		= std::max(ellipse.r.size.width, ellipse.r.size.height);

			// the ellipse axes should not differ too much in size
			if(big / small < 1.3) {

				ellipse.area	= cur_contour.size() + cv::contourArea(cur_contour);
				ellipse.n_rays	= (int)edges.size();

				ellipses.push_back(ellipse);

			}

		}

	}

	const size_t N = ellipses.size();

	if(N >= 4) {

		/*
		 *	A list of votes. Contains the number of votes obtained from autocorrelation
		 *	for each element. Example:
		 *		votes[0] : 5 means that ellipse 0 got 5 votes
		 */
		std::vector<VOTES> votes(ellipses.size());
		for(size_t i = 0; i < N; ++i) {
			votes[i] = VOTES(i, 0);
		}


		for(size_t i = 0; i < N; ++i) {

			// compare to this ellipse
			const cv::RotatedRect &r_target = ellipses[i].r;
			const double area_target = ellipses[i].area;

			// store the errors here
			std::vector<ERR> err_list;

			for(size_t j = 0; j < N; ++j) {

				// do not compare to self
				if(i == j) {continue;}

				// current ellipse to be compared to r_target
				const cv::RotatedRect &r_compare = ellipses[j].r;
				const double area_compare = ellipses[j].area;

				double err_area = std::max(area_target, area_compare) / std::min(area_target, area_compare) - 1.0;

				double diffx = std::max(r_target.size.width, r_compare.size.width) / std::min(r_target.size.width, r_compare.size.width);
				double diffy = std::max(r_target.size.height, r_compare.size.height) / std::min(r_target.size.height, r_compare.size.height);

				double diff2 = diffx * diffy;
				double err_axes = diff2 - 1.0;

				double err_rays = (double)N_ELLIPSE_RAYS / (double)ellipses[j].n_rays - 1.0;

				// perform the comparison
				double err = err_area + err_axes + err_rays;

				err_list.push_back(ERR(err, j));

			}

			// sort the errors in ascending order
			std::sort(err_list.begin(), err_list.end(), myobject);

			// vote for the 3 best matches
			++votes[err_list[0].index].n_votes;
			++votes[err_list[1].index].n_votes;
			++votes[err_list[2].index].n_votes;
		}

		// sort the votes in descending order
		std::sort(votes.begin(), votes.end(), sort_votes);

		for(size_t i = 0; i < 4; ++i) {

			int index = votes[i].index;

			const cv::RotatedRect &ell = ellipses[index].r;

			image_points.push_back(cv::Point2f(ell.center.x, ell.center.y));
		}

		organise_corners(image_points);

		return true;
	}

	return false;
}


bool fnc_left_first(cv::Point2f i, cv::Point2f j) {return (i.x < j.x);}


/*
 *         0___________ 3
 *          |         |
 *          |         |
 *          |         |
 *          |_________|
 *        1             2
 *
 *
 */
void organise_corners(std::vector<cv::Point2f> &corners) {

	// get the two left corners
	std::sort(corners.begin(), corners.end(), fnc_left_first);
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


void RectPattern::computeNormal() {

}


