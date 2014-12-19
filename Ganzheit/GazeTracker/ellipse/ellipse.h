#ifndef ELLIPSE_H
#define ELLIPSE_H


#include <opencv2/core/core.hpp>


namespace ellipse {

    /*
     * Get the ellipse perimeter point at the given angle. The angle
     * describes the angle in the ellipses, coordinate system and must
     * be expressed in radians. The function determines the intersection
     * between the ellipse and the line defined by the angle.
     *
     * http://math.stackexchange.com/questions/22064/calculating-a-point-that-lies-on-an-ellipse-given-an-angle
     *
     * An origin centered ellipse equation is
     *     x^2 / a^2 + y^2 / b^2 = 1.
     *
     * A line equation can be expressed as
     *     y = tan(t)*x
     *
     *              ==>
     *
     *     x^2 / a^2 + (tan(t)*x)^2 / b^2 = 1
     *
     *                  +- ab
     *     x = --------------------------
     *          sqrt(b^2 + a^2*tan(t)^2)
     *
     *     y = tan(t)*x
     *
     *     apply ellipse rotation
     *
     *
     */
    cv::Point2d getEllipsePoint(const cv::RotatedRect &ellipse, double dAngPointRad);


    /*
     * Get the ellipse perimeter point at the given angle. This function
     * uses the given angle to compute the x and y of the point as
     *
     * x = a*cos(ang)
     * y = b*sin(ang)
     * apply ellipse rotation
     *
     *    where a is the half of the width of the ellipse and b is half of
     *    the height of the ellipse.
     *
     * Therefore the given angle is not strictly speaking the angle at which the
     * point is found.
     *
     */
    cv::Point2d getEllipsePointNonPolar(const cv::RotatedRect &ellipse, double dAngPointRad);


    bool pointInsideEllipse(const cv::RotatedRect &ell, const cv::Point2f &p);


} // end of "namespace ellipse"



#endif

