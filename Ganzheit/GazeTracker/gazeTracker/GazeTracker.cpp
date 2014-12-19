#include <cmath>
#include "GazeTracker.h"
#include "trackerSettings.h"
#include <string.h>
#include "group.h"
#include "ellipse.h"

#include <sys/time.h>



static const double pi			= 3.14159265;
static const double pi_div_180	= 0.0174532925;
static const double _180_div_pi	= 57.2957795;


#define POW2(X)	((X)*(X))


#define debug(format, ...) printf (format, __VA_ARGS__)



namespace gt {


    static inline double RADTODEG(double rad) {
        return _180_div_pi * rad;
    }


    static inline double DEGTORAD(double deg) {
        return pi_div_180 * deg;
    }


    static void normalise(cv::Point3d &v) {

        double dLenInv = 1.0 / sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
        v *= dLenInv;

    }


    /*************************************************************************
     * The positions must be given this way
     *
     *               0
     *               o
     *
     *     1 o               o 5
     *
     *     2 o               o 4
     *
     *               o
     *               3
     *************************************************************************/
    GazeTracker::SixLEDs::SixLEDs(const std::vector<cv::Point3d> &positions) :
                          GazeTracker::LEDPattern(positions) {

        // set the LED positions
        leds.resize(6);
        for(int i = 0; i < 6; ++i) {
            leds[i].setPos(positions[i]);
        }

    }


    bool GazeTracker::SixLEDs::associateGlints(const std::vector<cv::Point2d> &glints,
                                               cv::Point2f pc,
                                               const std::vector<cv::Point3d> &glints_3d) {

        const size_t sz = glints.size();

        if(sz <= 1 || sz >= 7) {
            return false;
        }

        /************************************************************************
         * The GroupManager requires an integer coordinate vector. Therefore
         * the given double precision vector must be copied to such a container.
         ************************************************************************/
        std::vector<cv::Point> extracted(sz);
        {
            std::vector<cv::Point>::iterator eit = extracted.begin();
            std::vector<cv::Point2d>::const_iterator git = glints.begin();

            while(eit != extracted.end()) {
                eit->x = (int)(git->x + 0.5);
                eit->y = (int)(git->y + 0.5);
                ++eit;
                ++git;
            }
        }

        /************************************************************************
         * try to identify the glints
         ************************************************************************/
        group::GroupManager grp;
        if(!grp.identify(extracted, pc)) {
            return false;
        }

        /************************************************************************
         * Initialise the leds vector with null points
         ************************************************************************/
        {
            cv::Point3d null_vector(0, 0, 0);
            std::vector<LED>::iterator ledit = leds.begin();
            while(ledit != leds.end()) {
                ledit->setGlintDirection3D(null_vector);
                ledit->setGlint2D(cv::Point2d());
                ledit->setLabel(-1);
                ++ledit;
            }
        }

        /***********************************************************************
         * Ask the best configuration directly from the pattern finder
         **********************************************************************/
        {

            const group::Configuration &config = grp.getBestConfiguration();
            std::vector<group::Element>::const_iterator elit = config.elements.begin();
            std::vector<group::Element>::const_iterator end = config.elements.end();


            /************************************************************************
             * Set the identified glints for the leds, yes it is quite funny that a
             * container that supposedly stores LED info, also stores info about the
             * glint. Deal with it.
             ************************************************************************/
            int i = 0;
            while(elit != end) {

                const cv::Point3d &curGlint = glints_3d[i];

                /*
                 * The elements in std::vector<...> elements, are in the same order
                 * as the std::vector<...> glints_3d
                 */
                leds[elit->label].setGlintDirection3D(curGlint); // direction vector to the glint
                leds[elit->label].setGlint2D(glints[i]); // direction vector to the glint
                leds[elit->label].setLabel(elit->label);
                ++elit;
                ++i;

            }

        }

        return true;

    }


    void GazeTracker::SixLEDs::getCorneaCentre(Cornea *cornea, cv::Point3d &centreCornea) {

        std::vector<double> guesses;
        std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > glint_pos;
        std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > led_pos;

        for(int i = 0; i < 6; ++i) {

            const cv::Point3d *gl = leds[i].getGlintDirection3D();
            if(gl == NULL) {
                continue;
            }

            guesses.push_back(0);
            Eigen::Vector3d eigGlint(gl->x, gl->y, gl->z);
            glint_pos.push_back(eigGlint);

            Eigen::Vector3d eigLED(leds[i].pos().x, leds[i].pos().y, leds[i].pos().z);
            led_pos.push_back(eigLED);

        }

        double error;
        Eigen::Vector3d eigCentre;
        cornea->computeCentre(led_pos, glint_pos, guesses, eigCentre, error);

        centreCornea.x = eigCentre(0);
        centreCornea.y = eigCentre(1);
        centreCornea.z = eigCentre(2);

    }



    /********************************************************
     * class LED
     ********************************************************/

    LED::LED() {
        m_nLabel = -1;
    }


    const cv::Point3d &LED::pos(void) const {
        return m_posLED3D;
    }


    const cv::Point3d *LED::getGlintDirection3D() const {

        if(m_directionGlint3D.x == 0 &&
           m_directionGlint3D.y == 0 &&
           m_directionGlint3D.z == 0) {

            return NULL;

        }

        return &m_directionGlint3D;

    }



    /********************************************************
     * class GazeTracker
     ********************************************************/

    GazeTracker::GazeTracker() {

        // create the pupil tracker
        pupil_tracker = new PupilTracker();

        // create the cornea computer
        cornea = new Cornea();

        centre_pupil[0] = 0.0;
        centre_pupil[1] = 0.0;
        centre_pupil[2] = 0.0;

        centre_cornea[0] = 0.0;
        centre_cornea[1] = 0.0;
        centre_cornea[2] = 0.0;

        m_pPattern = NULL;

        track_dur_micros = -1;

        m_dPupilRadius = 0.0;

    }


    GazeTracker::~GazeTracker() {
        delete pupil_tracker;
        delete cornea;
        delete m_pPattern;
    }


    void GazeTracker::init(const Camera &camera,
                           const std::vector<cv::Point3d> &vecLEDs) {

        // copy the camera
        m_camera = camera;


        if(m_pPattern != NULL) {
            delete m_pPattern;
        }

        // create the pattern from the given LED position
        m_pPattern = new GazeTracker::SixLEDs(vecLEDs);

        int n = (int)(vecLEDs.size());
        pupil_tracker->set_nof_crs(n);
    }


    /*
     */
    bool GazeTracker::track(const cv::Mat &img) {

        // duration will be changed to a sensible value on success
        track_dur_micros = -1;

        struct timeval t1, t2;
        gettimeofday(&t1, NULL);

        reset();

        /*
         *================================================================================
         *================== Track the pupil, the glints and the cornea ==================
         *================================================================================
         */

        // track the pupil and the glints
        pupil_tracker->track(img);


        // get the glints
        const std::vector<cv::Point2d> glints = pupil_tracker->getCornealReflections();

        if(glints.size() < 2) {return false;}

        // convert the 2D glints to 3D direction vectors
        std::vector<cv::Point3d> glints_3d(glints.size());

        for(size_t i = 0; i < glints.size(); ++i) {

            const double x = glints[i].x;
            const double y = glints[i].y;

            m_camera.pixToWorld(x, y, glints_3d[i]);

            if(glints_3d[i].z <= 0) {	// must be in front of the camera
                printf("GazeTracker::track(): Camera::pixToWorld() failed, (%.2f, %.2f)\n", x, y);
                return false;
            }

        }


        const cv::RotatedRect *ellipse_pupil = pupil_tracker->getEllipsePupil();
        if(!m_pPattern->associateGlints(glints, ellipse_pupil->center, glints_3d)) {
            return false;
        }
        cv::Point3d cw;
        m_pPattern->getCorneaCentre(cornea, cw);


        centre_cornea[0] = cw.x;
        centre_cornea[1] = cw.y;
        centre_cornea[2] = cw.z;

        /*
         *================================================================================
         *======================== Compute the pupil centre in 3D ========================
         *================================================================================
         */

        const int NOF_PERIMETER_POINTS = trackerSettings.NOF_PERIMETER_POINTS;

        /*
         * In order to find out the centre of the pupil, we compute the
         * average of the pupil perimeter points. We must divide the points
         * into two sets, the upper and the lower points and find the points
         * in pairs. This is because if traceDirVecToPupil() fails for a point,
         * and succeeds for its pair point, the mass centre moves towards
         * the successfully computed point.
         */
        std::vector<cv::Point2d> cluster_up(NOF_PERIMETER_POINTS / 2);
        std::vector<cv::Point2d> cluster_down(NOF_PERIMETER_POINTS / 2);
        GazeTracker::getEllipsePoints(*ellipse_pupil, 0.0, -pi, cluster_up);
        GazeTracker::getEllipsePoints(*ellipse_pupil, pi, 0.0, cluster_down);


        /**************************************************************************
         *	Compute the distance rp, i.e. the distance between the pupil centre
         *	and max pupil perimeter.
         **************************************************************************/
        const double rp = getRP(cw);
        if(rp < 0.0) {
            return false;
        }

        // rd
        const double rd = trackerSettings.rd;

        // rps squared
        const double rps2 = rd*rd + rp*rp;

        double sum_u_hat_x	= 0.0;
        double sum_u_hat_y	= 0.0;
        double sum_u_hat_z	= 0.0;
        int c_valid			= 0;

        for(size_t i = 0; i < cluster_up.size(); ++i) {

            bool b_upper_point_found;
            cv::Point3d pupil_point_up;

            {
                cv::Point3d vec_k;
                const double u2D = cluster_up[i].x;
                const double v2D = cluster_up[i].y;
                m_camera.pixToWorld(u2D, v2D, vec_k);

                b_upper_point_found = traceDirVecToPupil(vec_k,
                                                         cw,
                                                         rps2,
                                                         pupil_point_up);
            }

            if(b_upper_point_found) {

                cv::Point3d vec_k;
                const double u2D = cluster_down[i].x;
                const double v2D = cluster_down[i].y;
                m_camera.pixToWorld(u2D, v2D, vec_k);
                cv::Point3d pupil_point_down;

                if(traceDirVecToPupil(vec_k,
                                      cw,
                                      rps2,
                                      pupil_point_down)) {

                    sum_u_hat_x += pupil_point_up.x + pupil_point_down.x;
                    sum_u_hat_y += pupil_point_up.y + pupil_point_down.y;
                    sum_u_hat_z += pupil_point_up.z + pupil_point_down.z;

                    c_valid += 2;

                    m_vec3DPerimeterPoints.push_back(pupil_point_up);
                    m_vec3DPerimeterPoints.push_back(pupil_point_down);

                }

            }

        } // for(size_t i = 0; i < cluster_up.size(); ++i) {

        if(c_valid > 0) {
            centre_pupil[0] = sum_u_hat_x / (double)c_valid;
            centre_pupil[1] = sum_u_hat_y / (double)c_valid;
            centre_pupil[2] = sum_u_hat_z / (double)c_valid;

            //		return true;
        }
        else {
            return false;
        }


        // compute the pupil radius from the perimeter points
        computePupilRadius();

        gettimeofday(&t2, NULL);

        const long usec = t2.tv_usec - t1.tv_usec;
        const long sec = t2.tv_sec - t1.tv_sec;
        track_dur_micros = sec * 1000000 + usec;

        return true;

    }


    void GazeTracker::computePupilRadius() {

        std::list<cv::Point3d>::iterator it    = m_vec3DPerimeterPoints.begin();
        std::list<cv::Point3d>::iterator itEnd = m_vec3DPerimeterPoints.end();
        const size_t nSz = m_vec3DPerimeterPoints.size();

        // sum of distances between the mass centre and the perimeter points
        double dSum = 0.0;

        // compute the average distance to the mass centre
        while(it != itEnd) {

            // current perimeter point
            const cv::Point3d &cur3DPoint = *it;

            // difference between the mass centre and the perimeter point
            const double dX = cur3DPoint.x - centre_pupil[0];
            const double dY = cur3DPoint.y - centre_pupil[1];
            const double dZ = cur3DPoint.z - centre_pupil[2];

            // accumulate
            dSum += sqrt(dX*dX + dY*dY + dZ*dZ);

            ++it;

        }

        // average
        m_dPupilRadius = dSum / nSz;

    }


    void GazeTracker::reset() {

        centre_pupil[0] = 0.0;
        centre_pupil[1] = 0.0;
        centre_pupil[2] = 0.0;

        centre_cornea[0] = 0.0;
        centre_cornea[1] = 0.0;
        centre_cornea[2] = 0.0;

        m_vec3DPerimeterPoints.clear();
        m_dPupilRadius = 0.0;

    }


    double GazeTracker::getRP(const cv::Point3d &cw) {

        const cv::RotatedRect *ellipse_pupil = pupil_tracker->getEllipsePupil();

        double ang[2];
        if(ellipse_pupil->size.width > ellipse_pupil->size.height) {	// horisontal axis is the major axis
            ang[0] = 0.0;
            ang[1] = pi;
        }
        else {	// vertical axis is the major axis
            ang[0] = pi / 2.0;
            ang[1] = -pi / 2.0;
        }

        double major_start[3];
        double major_end[3];
        double *major[2] = {major_start, major_end};


        double c_x = cw.x;
        double c_y = cw.y;
        double c_z = cw.z;

        double c_x2 = POW2(c_x);
        double c_y2 = POW2(c_y);
        double c_z2 = POW2(c_z);

        // cornea ball radius squared
        double rho2 = POW2(trackerSettings.RHO);

        for(int i = 0; i < 2; ++i) {
            //cv::Point2d p2D;
            //GazeTracker::getEllipsePoint(*ellipse_pupil, ang[i], p2D);
            cv::Point2d p2D = ellipse::getEllipsePoint(*ellipse_pupil, ang[i]);

            cv::Point3d p3D;
            m_camera.pixToWorld(p2D.x, p2D.y, p3D);

            // the direction vector towards the pupil perimeter point
            double K_x = p3D.x;
            double K_y = p3D.y;
            double K_z = p3D.z;

            // square it
            double K_x2 = POW2(K_x);
            double K_y2 = POW2(K_y);
            double K_z2 = POW2(K_z);

            // helpers for obtaining the length s
            double a = K_x2 + K_y2 + K_z2;
            double b = -2.0 * (c_x*K_x + c_y*K_y + c_z*K_z);
            double c = c_x2 + c_y2 + c_z2 - rho2;

            const double discriminant = POW2(b) - 4.0*a*c;
            if(discriminant < 0.0) {
                return -1.0;
            }

            const double sqrt_discriminant = std::sqrt(discriminant);

            // the candidates for s
            double s1 = (-b + sqrt_discriminant) / (2.0*a);
            double s2 = (-b - sqrt_discriminant) / (2.0*a);

            double s = std::min(s1, s2);

            if(s < 0.0) {
                return -1.0;
            }

            major[i][0] = s*K_x;
            major[i][1] = s*K_y;
            major[i][2] = s*K_z;
        }


        double x = major_end[0] - major_start[0];
        double y = major_end[1] - major_start[1];
        double z = major_end[2] - major_start[2];

        double rp = std::sqrt(x*x + y*y + z*z) / 2.0;

        if(rp > trackerSettings.RHO) {
            printf("GazeTracker::getRP(): rp > RHO!!!\n");
            return -1.0;
        }

        return rp;

    }


    bool GazeTracker::traceDirVecToPupil(const cv::Point3d &dir_vec, // the direction vector
                                         const cv::Point3d &cw,      // the cornea centre
                                         double rps2,                // the squared distance from cw to the pupil perimeter point
                                         cv::Point3d &pupil_point) { // the resulting point on the pupil perimeter

        /********************************************************************
         * Get u
         ********************************************************************/

        // the direction vector of the pupil perimeter point
        const double K_x = dir_vec.x;
        const double K_y = dir_vec.y;
        const double K_z = dir_vec.z;

        // square it
        const double K_x2 = POW2(K_x);
        const double K_y2 = POW2(K_y);
        const double K_z2 = POW2(K_z);

        // cornea sphere radius
        const double RHO = trackerSettings.RHO;

        // MYY = n_air / n_cornea ~= 0.75
        const double MYY = trackerSettings.MYY;

        // cornea sphere radius squared
        const double rho2 = POW2(RHO);

        const double cx = cw.x;
        const double cy = cw.y;
        const double cz = cw.z;

        const double cx2 = cx*cx;
        const double cy2 = cy*cy;
        const double cz2 = cz*cz;

        // helpers for obtaining the length s
        double a = K_x2 + K_y2 + K_z2;
        double b = -2.0 * (cx*K_x + cy*K_y + cz*K_z);
        double c = cx2 + cy2 + cz2 - rho2;

        double discriminant = b*b - 4.0*a*c;
        if(discriminant < 0) {
            return false;
        }


        // the candidates for s
        double sqrt_discriminant = std::sqrt(discriminant);
        const double s1 = (-b + sqrt_discriminant) / (2.0*a);
        const double s2 = (-b - sqrt_discriminant) / (2.0*a);

        /*
         * Two solutions: One that intersects the sphere at the 
         * closer surface and the other that intersects the
         * sphere at the farther, back, surface. We want the 
         * closer intersection.
         */
        const double s = std::min(s1, s2);

        if(s < 0.0) {return false;}

        // point on the cornea surface
        cv::Point3d u = s*dir_vec;


        /********************************************************************
         * Get û
         ********************************************************************/

        // use Snell's law http://en.wikipedia.org/wiki/Snell's_law

        // compute the normal of the surface at the current u
        cv::Point3d N = u - cw;
        normalise(N);


        const cv::Point3d l = dir_vec;

        const double cos_theta1		= N.dot(-l);
        const double cos_theta2		= std::sqrt(1.0 - POW2(MYY) * (1.0 - POW2(cos_theta1)));
        const double sign			= cos_theta1 >= 0 ? 1.0 : -1.0;
        cv::Point3d K_hat	= (MYY*l + (MYY * cos_theta1 - sign * cos_theta2)*N);
        normalise(K_hat);

        const double K_hat_x = K_hat.x;
        const double K_hat_y = K_hat.y;
        const double K_hat_z = K_hat.z;

        // square
        const double K_hat_x2 = POW2(K_hat_x);
        const double K_hat_y2 = POW2(K_hat_y);
        const double K_hat_z2 = POW2(K_hat_z);

        const double u_x = u.x;
        const double u_y = u.y;
        const double u_z = u.z;

        // square
        const double u_x2 = POW2(u_x);
        const double u_y2 = POW2(u_y);
        const double u_z2 = POW2(u_z);


        a = K_hat_x2 + K_hat_y2 + K_hat_z2;
        b = 2.0 * (u_x*K_hat_x + u_y*K_hat_y + u_z*K_hat_z - cx*K_hat_x - cy*K_hat_y - cz*K_hat_z);
        c = u_x2 + u_y2 + u_z2 - 2.0 * (cx*u_x + cy*u_y + cz*u_z) + cx2 + cy2 + cz2 - rps2;

        discriminant = b*b - 4.0*a*c;
        if(discriminant < 0) {
            return false;
        }

        sqrt_discriminant = std::sqrt(discriminant);
        const double w1 = (-b + sqrt_discriminant) / (2.0*a);
        const double w2 = (-b - sqrt_discriminant) / (2.0*a);

        const double w = std::min(w1, w2);

        if(w < 0.0) {return false;}

        pupil_point = u + w * K_hat;

        return true;

    }


    void GazeTracker::getEllipsePoints(const cv::RotatedRect &ellipse,
                                       double ang_p_start,
                                       double ang_p_end,
                                       std::vector<cv::Point2d> &points) {

        if(points.size() == 1) {
            points[0] = ellipse::getEllipsePoint(ellipse, ang_p_start);
            return;
        }

        const double inc_ang_p = (ang_p_end - ang_p_start) / (double)(points.size()-1.0);
        double ang_p = ang_p_start;

        for(size_t i = 0; i < points.size(); ++i) {
            points[i] = ellipse::getEllipsePoint(ellipse, ang_p);
            ang_p += inc_ang_p;
        }

    }


    void GazeTracker::getEllipsePoints(const cv::RotatedRect &ellipse,
                                       std::vector<cv::Point2d> &points) {

        GazeTracker::getEllipsePoints(ellipse, 0.0, 2.0*pi, points);

    }


}	// end of namespace gt {

