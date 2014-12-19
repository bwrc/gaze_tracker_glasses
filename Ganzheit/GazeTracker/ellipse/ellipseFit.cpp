#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/LU>
#include "ellipse.h"
#include <math.h>
#include <float.h>


#define IS_FINITE(X)	(((X) <= DBL_MAX && (X) >= -DBL_MAX))
#define pi				3.1415926535897932384626433832795


namespace gt {


USING_PART_OF_NAMESPACE_EIGEN



namespace Ellipse {


/*
 *	Copied from Mika Letonsaari's Master's Thesis.
 *	Z:\Science\Datam_projektit\Monitehtävä\TekesRaportti\Opinnäytetyöt\Letonsaari_Diplomityö\Softa\Koodi
 */
double fitEllipse(const std::vector<gt::MY_POINT> &edges, Ellipse::MY_ELLIPSE &ret) {
	int size = (int)edges.size();

	Eigen::MatrixXd d1(3, size);
	Eigen::MatrixXd d2(3, size);

	for(int i = 0; i < size; ++i) {
		// Quadratic part of the design matrix
		d1(0, i) = (double)edges[i].x * edges[i].x;
		d1(1, i) = (double)edges[i].x * edges[i].y;
		d1(2, i) = (double)edges[i].y * edges[i].y;

		// Linear part of design matrix
		d2(0, i) = (double)edges[i].x;
		d2(1, i) = (double)edges[i].y;
		d2(2, i) = 1.0;
	}

	// quadratic scatter
	static Eigen::Matrix3d s1;
	for(int i = 0; i <= 2; ++i) {
		for(int j = 0; j <= 2; ++j) {
		  double sum = 0;
			for(int k = 0; k < size; ++k) {
				sum += d1(i, k) * d1(j, k);
			}
			s1(i, j) = sum;
		}
	}

	// combined scatter
	static Eigen::Matrix3d s2;
	for(int i = 0; i <= 2; ++i) {
		for(int j = 0; j <= 2; ++j) {
			double sum = 0;
			for(int k = 0; k < size; ++k) {
				sum += d1(i, k) * d2(j, k);
			}
			s2(j, i) = sum;
		}
	}

	// linear scatter
	static Eigen::Matrix3d s3;
	for(int i = 0; i <= 2; ++i) {
		for(int j = 0; j <= 2; ++j) {
			double sum = 0;
			for(int k = 0; k < size; ++k) {
				sum += d2(i, k) * d2(j, k);
			}
			s3(i, j) = sum;
		}
	}

	// compute the inverse of s3 into the temp variable
	static Eigen::Matrix3d s3_inv;
	s3.computeInverse(&s3_inv);

	if(!IS_FINITE(s3_inv(0, 0)) || !IS_FINITE(s3_inv(1, 0)) || !IS_FINITE(s3_inv(2, 0)) ||
	   !IS_FINITE(s3_inv(0, 1)) || !IS_FINITE(s3_inv(1, 1)) || !IS_FINITE(s3_inv(2, 1)) ||
	   !IS_FINITE(s3_inv(0, 2)) || !IS_FINITE(s3_inv(1, 2)) || !IS_FINITE(s3_inv(2, 2))) {

		memset(&ret, 0, sizeof(MY_ELLIPSE));
		return 10000.0;
	}

	static Eigen::Matrix3d t;

	// getting a2 from a1
	for(int i = 0; i <= 2; ++i) {
		for(int j = 0; j <= 2; ++j) {
			double sum = 0;
			for(int k = 0; k <= 2; ++k) {
				sum -= s3_inv(i, k) * s2(k, j);
			}
			t(j, i) = sum;
		}
	}

	// reduced scatter matrix
	static Eigen::Matrix3d m;

	for(int i = 0; i <= 2; ++i) {
		for(int j = 0; j <= 2; ++j) {
			double sum = 0;
			for(int k = 0; k <= 2; ++k) {
				sum += s2(k, i) * t(j, k);
			}
			m(j, i) = sum + s1(j, i);
		}
	}

	static Eigen::Matrix3d m2;
	for(int i = 0; i <= 2; ++i) {
		m2(0, i) = m(2, i) / 2;
		m2(1, i) = -m(1, i);
		m2(2 ,i) = m(0, i) / 2;
	}

	/* Exit if all elements are zero, because otherwise the Eigensolver jams */
	if(m2(0, 0) == 0 && m2(0, 1) == 0 && m2(0, 2) == 0 &&
	   m2(1, 0) == 0 && m2(1, 1) == 0 && m2(1, 2) == 0 &&
	   m2(2, 0) == 0 && m2(2, 1) == 0 && m2(2, 2) == 0) {

		memset(&ret, 0, sizeof(MY_ELLIPSE));
		return 10000.0;
	}

	// solve the eigen vector
	Eigen::EigenSolver<Matrix3d> eig(m2);
	static Eigen::Matrix3d evec;
	evec = eig.eigenvectors().real();

	static Eigen::Vector3d cond(3);


	/* evaluate a'Ca

		a1 = a1' * C1 * a1 = 1, where a1' = [a, b, c] => 4ac-b2=1

				 |0	 0	2|
			C1 = |0	-1	0|
				 |2	 0	0|
	*/

	for(int i = 0; i <= 2; ++i) {
		cond(i) = 4 * evec(0, i) * evec(2, i) - evec(1, i) * evec(1, i);
	}

	static double a[6];

	double err = 1000000.0;

	/* En ole varma onko tässä mitään järkeä. Mitä jos cond -vektorissa
	   onkin useampi positiivinen luku? T: Sharman */
	// a[0..2] = ominaisvektori, jolla pienin vastaava cond-arvo
	for(int i = 0; i <= 2; ++i) {
		if(cond(i) > 0 && cond(i) < err) {
			for(int j = 0; j <= 2; ++j) {
				a[j] = evec(j, i);
			}
			err = std::abs(1.0 - cond(i));
		}
	}

	// reduced scatter matrix
	for(int i = 0; i <= 2; ++i) {
		double sum = 0;
		for(int k = 0; k <= 2; ++k) {
			sum += t(k, i) * a[k];
		}
		a[i + 3] = sum;
	}

	Ellipse::formEllipse(a, ret);

	return err;
}


/* http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/FISHER/ELLIPFIT/drawellip.m
   http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/FISHER/ELLIPFIT/solveellipse.m
   draws an ellipse with a[0] * x^2 + a[1] * x * y + a[2] * y^2 + a[3] * x + a[4] * y + a[5] = 0
   overlaid with original points edge */
void formEllipse(double *a, Ellipse::MY_ELLIPSE &ret) {
	/* -------------------------STEP 1:-------------------------
		Given an ellipse in the form:
			a(1)x^2 + a(2)xy + a(3)y^2 + a(4)x + a(5)y + a(6) = 0
		finds the standard form:
			((x-cx)/r1)^2 + ((y-cy)/r2)^2 = 1
	*/

	// get ellipse orientation
	double theta = atan(a[1] / (a[0] - a[2])) / 2.0;

	// get scaled major/minor axes
	double ct = cos(theta);
	double st = sin(theta);
	double ap = a[0] * ct * ct + a[1] * ct * st + a[2] * st * st;
	double cp = a[0] * st * st - a[1] * ct * st + a[2] * ct * ct;

	// get translations
	Eigen::Matrix2d T;
	T(0, 0) = a[0];
	T(1, 0) = a[1] / 2.0;
	T(0, 1) = a[1] / 2.0;
	T(1, 1) = a[2];

	Eigen::Matrix2d temp = 2.0 * T;
	Eigen::Vector2d temp_vect;
	temp_vect << a[3], a[4];

	Eigen::MatrixXd t = -1.0 * temp.inverse() * temp_vect;
	double cx = t(0, 0);
	double cy = t(1, 0);

	// get scale factor
	double val = (t.transpose() * T * t)(0, 0);
	double scale = 1.0 / (val - a[5]);

	// get major/minor axis radii
	double r1 = 1.0 / sqrt(scale * ap);
	double r2 = 1.0 / sqrt(scale * cp);
	//double v[5] = {r1, r2, cx, cy, theta};

	ret.r1		= r1;
	ret.r2		= r2;
	ret.cx		= cx;
	ret.cy		= cy;
	ret.theta	= theta;
}


void getEllipsePoint(const Ellipse::MY_ELLIPSE &ellipse, gt::MY_POINT2d &ret, double ang) {
	Eigen::Matrix2d	R;
	R(0, 0) = cos(ellipse.theta);
	R(1, 0) = sin(ellipse.theta);
	R(0, 1) = -sin(ellipse.theta);
	R(1, 1) = cos(ellipse.theta);

	Eigen::MatrixXd temp_col(2, 1);
	double x = ellipse.r1 * cos(ang);
	double y = ellipse.r2 * sin(ang);
	temp_col << x, y;
	Eigen::MatrixXd d1 = R * temp_col;
	ret.x = d1(0, 0) + ellipse.cx;
	ret.y = d1(1, 0) + ellipse.cy;
}


void getEllipsePoints(const Ellipse::MY_ELLIPSE &ellipse, std::vector<gt::MY_POINT2d> &ret) {
	int N = (int)ret.size();

	// draw ellipse with N points
	double dx = 2.0 * pi / N;

	Eigen::Matrix2d	R;
	R(0, 0) = cos(ellipse.theta);
	R(1, 0) = sin(ellipse.theta);
	R(0, 1) = -sin(ellipse.theta);
	R(1, 1) = cos(ellipse.theta);

	Eigen::MatrixXd temp_col(2, 1);
	for(int i = 0; i < N; ++i) {
		double ang = i * dx;
		double x = ellipse.r1 * cos(ang);
		double y = ellipse.r2 * sin(ang);
		temp_col << x, y;
		Eigen::MatrixXd d1 = R * temp_col;
		ret[i].x = d1(0, 0) + ellipse.cx;
		ret[i].y = d1(1, 0) + ellipse.cy;
	}
}


/* Gets the points of the given ellipse within the given range. Also returns
   the points on the other side of the ellipse. So there are two ranges
   [ang1, ang2] and [-ang1, -ang2]. */
void getEllipsePoints(const MY_ELLIPSE &ellipse,
					  std::vector<gt::MY_POINT2d> &ret,
					  double ang1,	// start angle in degrees
					  double ang2) {	// end angle in degrees

	int N = (int)ret.size() / 2.0;

	// convert the angles into radians
	ang1 = ang1 * pi / 180.0;
	ang2 = ang2 * pi / 180.0;

	// draw ellipse with N points
	double dx = (ang2 - ang1) / N;

	Eigen::Matrix2d	R;
	R(0, 0) = cos(ellipse.theta);
	R(1, 0) = sin(ellipse.theta);
	R(0, 1) = -sin(ellipse.theta);
	R(1, 1) = cos(ellipse.theta);


	// for the first range
	double ang = ang1;
	Eigen::MatrixXd temp_col(2, 1);
	for(int i = 0; i < N; ++i) {
		double x = ellipse.r1 * cos(ang);
		double y = ellipse.r2 * sin(ang);
		temp_col << x, y;
		Eigen::MatrixXd d1 = R * temp_col;
		ret[i].x = d1(0, 0) + ellipse.cx;
		ret[i].y = d1(1, 0) + ellipse.cy;
		ang += dx;
	}

	// for the second range
	ang = ang1 + pi;
	for(int i = N; i < 2 * N; ++i) {
		double x = ellipse.r1 * cos(ang);
		double y = ellipse.r2 * sin(ang);
		temp_col << x, y;
		Eigen::MatrixXd d1 = R * temp_col;
		ret[i].x = d1(0, 0) + ellipse.cx;
		ret[i].y = d1(1, 0) + ellipse.cy;
		ang += dx;
	}
}


}	// end of namespace Ellipse {


}	// end of namespace gt {

