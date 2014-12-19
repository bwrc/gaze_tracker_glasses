#include "ellipse.h"


namespace ellipse {


    static const double dPi       = 3.1415926535897932384626433832795;
    static const double dTwoPi    = 2.0*dPi;
    static const double dPiPer180 = dPi / 180;
    static const double dPiPer2   = dPi / 2.0;


    cv::Point2d getEllipsePointNonPolar(const cv::RotatedRect &ellipse, double dAngPointRad) {

        const double r1 = ellipse.size.width  * 0.5;
        const double r2 = ellipse.size.height * 0.5;

        const double x = r1 * cos(dAngPointRad);
        const double y = r2 * sin(dAngPointRad);

        // angle of the ellipse
        const double dAngEllRad = ellipse.angle * dPiPer180;
        const double cos_ang = cos(dAngEllRad);
        const double sin_ang = sin(dAngEllRad);

        const double R[4] = {cos_ang, -sin_ang,
                             sin_ang, cos_ang};

        const double x_rot = R[0] * x + R[1] * y;
        const double y_rot = R[2] * x + R[3] * y;

        return cv::Point2d(x_rot + ellipse.center.x,
                           y_rot + ellipse.center.y);

    }


    cv::Point2d getEllipsePoint(const cv::RotatedRect &ellipse, double dAngPointRad) {

        if(ellipse.size.width == 0.0 || ellipse.size.height == 0.0) {
            return cv::Point2d();
        }

        // restrict within [0, 360]
        dAngPointRad = fmod(dAngPointRad, dTwoPi);

        // restrict within [-180, 180]
        if(fabs(dAngPointRad) > dPi) {
            const double dSign = dAngPointRad > 0 ? 1.0 : -1.0;
            dAngPointRad = dAngPointRad - dSign*dTwoPi;
        }

        // the coordinate in the ellipses coordinate system.
        double x = 0.0;
        double y = 0.0;

        if(fabs(dAngPointRad) == dPiPer2) {
            const double dSign = dAngPointRad >= 0.0 ? 1.0 : -1.0;
            y = dSign * ellipse.size.height * 0.5;
        }
        else {

            /*
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
             */

            const double a = ellipse.size.width  * 0.5;
            const double b = ellipse.size.height * 0.5;

            const double tana   = tan(dAngPointRad);

            const double dSign = (dAngPointRad > -dPiPer2 && dAngPointRad < dPiPer2) ? 1.0 : -1.0;

            x = dSign * (a*b) / sqrt(b*b + a*a*tana*tana);
            y = tana*x;

        }

        // angle of the ellipse
        const double dAngEllRad = ellipse.angle * dPiPer180;
        double cos_ang = cos(dAngEllRad);
        double sin_ang = sin(dAngEllRad);

        double R[4] = {cos_ang, -sin_ang,
                       sin_ang, cos_ang};

        double x_rot = R[0] * x + R[1] * y;
        double y_rot = R[2] * x + R[3] * y;

        return cv::Point2d(x_rot + ellipse.center.x,
                           y_rot + ellipse.center.y);


    }


    /*
     * (x - cx)^2     (y - cy)^2
     * ----------- +  ----------- <= 1
     *  semimaj^2      semimin^2
     */
    bool pointInsideEllipse(const cv::RotatedRect &ell, const cv::Point2f &p) {

        // ellipse centre
        const double dCx = ell.center.x;
        const double dCy = ell.center.y;

        // first express the point relative to the ellipse centre
        const double dX = p.x - dCx;
        const double dY = p.y - dCy;

        // then the point needs to be rotated
        const double dAngRad = -ell.angle * dPiPer180;
        const double dCos    = cos(dAngRad);
        const double dSin    = sin(dAngRad);

        const double pdRot[4] = {dCos, -dSin,
                                 dSin, dCos};

        const double dXRot = pdRot[0] * dX + pdRot[1] * dY;
        const double dYRot = pdRot[2] * dX + pdRot[3] * dY;

        const double dW = ell.size.width;
        const double dH = ell.size.height;


        const double dLeft  = (dXRot*dXRot) / (0.25*dW*dW);
        const double dRight = (dYRot*dYRot) / (0.25*dH*dH);

        return dLeft + dRight < 1.0;

    }



} // end of "namespace ellipse"

