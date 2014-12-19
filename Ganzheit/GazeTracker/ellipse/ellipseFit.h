#ifndef ELLIPSE_FIT_H
#define ELLIPSE_FIT_H


#include "structs.h"
#include "clusteriser.h"



namespace gt {

namespace Ellipse {


typedef struct MY_ELLIPSE {
	double r1;
	double r2;
	double cx;
	double cy;
	double theta;
} MY_ELLIPSE;


/*
 *	Copied from Mika Letonsaari's Master's Thesis.
 *	Z:\Science\Datam_projektit\Monitehtävä\TekesRaportti\Opinnäytetyöt\Letonsaari_Diplomityö\Softa\Koodi
 */
double fitEllipse(const std::vector<gt::MY_POINT> &edges, Ellipse::MY_ELLIPSE &ret);


/*
 *	http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/FISHER/ELLIPFIT/drawellip.m
 *	http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/FISHER/ELLIPFIT/solveellipse.m
 *	draws an ellipse with a[0] * x^2 + a[1] * x * y + a[2] * y^2 + a[3] * x + a[4] * y + a[5] = 0
 *	overlaid with original points edge
 */
void formEllipse(double *a, Ellipse::MY_ELLIPSE &ret);


void getEllipsePoint(const Ellipse::MY_ELLIPSE &ellipse, gt::MY_POINT2d &ret, double ang);


void getEllipsePoints(const MY_ELLIPSE &ellipse, std::vector<gt::MY_POINT2d> &ret);


/*
 *	Gets the points of the given ellipse within the given range. Also returns
 *	the points on the other side of the ellipse. So there are two ranges
 *	[ang1, ang2] and [-ang1, -ang2].
 */
void getEllipsePoints(	const Ellipse::MY_ELLIPSE &ellipse,
						std::vector<gt::MY_POINT2d> &ret,
						double ang1,	// start angle in degrees
						double ang2);	// end angle in degrees


}	// end of namespace Ellipse {


} // end of namespace gt {

#endif

