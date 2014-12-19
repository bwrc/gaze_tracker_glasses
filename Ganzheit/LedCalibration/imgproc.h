#ifndef IMGPROG_H
#define IMGPROG_H


#include <Eigen/SVD> 
#include <Eigen/Geometry>
#include <Eigen/LU>
#include <Eigen/SVD>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include <vector>


namespace imgproc {


typedef bool (*comparator)(unsigned char p1, unsigned char p2);


void findFromLine(const cv::Mat &img_gray,
				  int x0,
				  int y0,
				  int x1,
				  int y1,
				  unsigned char threshold,
				  cv::Point &ret_next,
				  cv::Point &ret_prev,
				  comparator comp = NULL);

void setEndPoints(const int x1,
				  const int y1,
				  int &x2,
				  int &y2,
				  const int distX,
				  const int distY,
				  const int img_w,
				  const int img_h);


//void getTransform(std::vector<cv::Point3f> &regpoint1,
//				  std::vector<cv::Point3f> &regpoint2,
//				  Eigen::Matrix4d &transform);



double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0);
bool getRectCorners(const cv::Mat &img_gray, std::vector<cv::Point2f> &corners);


template <class anyPoint>
void getCOM(const std::vector<anyPoint> &contour, cv::Point2f &com) {
	double x = 0;
	double y = 0;

	for(size_t i = 0; i < contour.size(); ++i) {
		x += contour[i].x;
		y += contour[i].y;
	}

	com.x = round(x/(double)contour.size());
	com.y = round(y/(double)contour.size());
}


void removeOutliers(std::vector<cv::Point> &edges, double var_coeff);
void removeOutliers(std::vector<double> &arr);


}


#endif

