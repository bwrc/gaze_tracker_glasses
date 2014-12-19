#include <Eigen/StdVector>
#include "LEDCalibrator.h"
#include <list>
#include <iostream>
#include "LEDTracker.h"
#include "LEDCalibPattern.h"

#include <gsl/gsl_multimin.h>


static const unsigned int  MAX_ITER = 10000;

using namespace std;

namespace calib {


    static bool getGlintCenter(cv::Mat &img_gray,
                               LEDCalibSample &resSample,
                               unsigned char thCr);



    class DataForGSL {

        public:

        // http://eigen.tuxfamily.org/dox-devel/group__TopicStructHavingEigenMembers.html
        EIGEN_MAKE_ALIGNED_OPERATOR_NEW

        DataForGSL(int n) {

            vectorS.resize(n);
            vectorSrn.resize(n);

        }

        std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > vectorS;
        std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > vectorSrn;

    };


    static double my_f(const gsl_vector *v, void *params);



    bool getGlintCenter(cv::Mat &img_gray,
                        LEDCalibSample &resSample,
                        unsigned char thCr) {

        cv::Point2d &gp = resSample.glint;
        double &gx = gp.x;
        double &gy = gp.y;
        const std::vector<cv::Point2f> &image_points = resSample.image_points;


        /********************************************
         * Get the corners of this pattern:
         *
         *          15  14   13
         *     0 o   o   o   o   o 12
         *
         *     1 o               o 11
         *
         *     2 o               o 10
         *
         *     3 o               o 9
         *
         *     4 o   o   o   o   o 8
         *           5   6   7
         ********************************************/
        std::vector<cv::Point2f> corners(4);
        corners[0] = image_points[0];
        corners[1] = image_points[4];
        corners[2] = image_points[8];
        corners[3] = image_points[12];

        LEDTracker LED_tracker;
        cv::Rect drawable_ellipse;
        bool found = LED_tracker.track(img_gray, corners, thCr, gx, gy, drawable_ellipse);

        return found;

    }


    bool extractFeatures(const cv::Mat &img_rgb,
                         LEDCalibSample &resSample,
                         unsigned char thRect,
                         unsigned char thCr) {

        // clear the samples
        resSample.clear();

        // grayscale image
        cv::Mat img_gray;

        // convert the rgb-image to a grayscale image
        cv::cvtColor(img_rgb, img_gray, CV_RGB2GRAY);


        /***************************************************************************
         * Track the pattern
         ***************************************************************************/

        // std::vector<cv::Point3f> object_points;

        if(!LEDCalibPattern::findMarkers(img_gray,
                                         thRect,
                                         resSample.image_points)) {

            resSample.clear();

            //            strErr = "Could not find markers";

            return false;

        }


        /***************************************************************************
         * Get the glint centre
         ***************************************************************************/
        if(!getGlintCenter(img_gray, resSample, thCr)) {

            resSample.clear();

            return false;

        }


        return true;

    }


    bool calibrateLED(const LEDCalibContainer &_container,
                      const Camera &_cam) {

        // sanity check
        if(_container.getSamples().size() < 2) {

            return false;

        }


        const std::vector<LEDCalibSample> &samples = _container.getSamples();

        // the number of samples
        const size_t sz = samples.size();

        DataForGSL gslData(sz);


        std::vector<cv::Point3f> objectPoints;
        LEDCalibPattern::getLEDObjectPoints(_container.circleSpacing,
                                            objectPoints);


        for(size_t i = 0; i < sz; ++i) {

            Eigen::Matrix4d transformation;

            // compute the transformation matrix
            LEDCalibPattern::makeTransformationMatrix(_cam.getIntrisicMatrix(),
                                                      _cam.getDistortion(),
                                                      samples[i].image_points,
                                                      objectPoints,
                                                      transformation);

            // compute the normal
            Eigen::Vector3d eig_normal;
            LEDCalibPattern::computeNormal(transformation, eig_normal);

            const Eigen::Vector3d S = computeIntersectionPoint(samples[i].glint,
                                                               objectPoints,
                                                               eig_normal,
                                                               transformation,
                                                               _cam);


            const Eigen::Vector3d Srn = computeReflectionPoint(eig_normal,
                                                               S);

            gslData.vectorS[i]   = S;
            gslData.vectorSrn[i] = Srn;

        }


        /* Starting point */
        gsl_vector *x = gsl_vector_alloc(3);
        gsl_vector_set(x, 0, 10.0);
        gsl_vector_set(x, 1, 10.0);
        gsl_vector_set(x, 2, 10.0);

        /* Set initial step sizes to 1 */
        gsl_vector *ss = gsl_vector_alloc(3);
        gsl_vector_set_all(ss, 1.0);

        /* Initialize method and iterate */
        gsl_multimin_function minex_func;
        minex_func.n = 3;
        minex_func.f = &my_f;
        minex_func.params = (void *)&gslData;

        const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex2;

        gsl_multimin_fminimizer *s = gsl_multimin_fminimizer_alloc(T, 3);
        gsl_multimin_fminimizer_set(s, &minex_func, x, ss);


        size_t iter = 0;
        int status;

        do {

            ++iter;
            status = gsl_multimin_fminimizer_iterate(s);

            if(status) {
                break;
            }

            double size = gsl_multimin_fminimizer_size(s);
            status = gsl_multimin_test_size(size, 1e-6);

        } while(status == GSL_CONTINUE && iter < MAX_ITER);

        printf("LEDCalibrator::calibrate(): iterations: %d\n", (int)iter);

        _container.LED_pos[0] = gsl_vector_get(s->x, 0);
        _container.LED_pos[1] = gsl_vector_get(s->x, 1);
        _container.LED_pos[2] = gsl_vector_get(s->x, 2);


        gsl_vector_free(x);
        gsl_vector_free(ss);
        gsl_multimin_fminimizer_free(s);

        if(status != GSL_SUCCESS) {
            return false;
        }

        return true;

    }


    double my_f(const gsl_vector *v, void *params) {

        const DataForGSL *gslData = (DataForGSL *)params;

        const double x = gsl_vector_get(v, 0);
        const double y = gsl_vector_get(v, 1);
        const double z = gsl_vector_get(v, 2);
        double error   = 0.0;

        // get the calibration data
        const std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > &vectorS   = gslData->vectorS;
        const std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > &vectorSrn = gslData->vectorSrn;

        size_t sz = vectorS.size();


        /*
         *
         * Mirror surface
         * _________________________________
         *             /\
         *           .   \
         *    P-S   /     \ S
         *        .        \
         *       /          \
         *     .              O
         *    /
         *   P
         *
         * O   = camera origin
         * P   = position of the LED
         * S   = vector from the camera to the mirror surface
         * Srn = direction vector from the mirror to the LED.
         *
         * If two vectors, A and B, are parallel, their cross product
         * is a zero vector:
         *
         *     A x B = 0i + 0j + 0k
         *
         * Therefore the cross product of P - S and Srn must be a zero vector
         */

        for(size_t i = 0; i < sz; ++i) {

            const Eigen::Vector3d &S   = vectorS[i];
            const Eigen::Vector3d &Srn = vectorSrn[i];

            double error_x = Srn(1) * (z - S(2)) - Srn(2) * (y - S(1)); error_x *= error_x;
            double error_y = Srn(0) * (z - S(2)) - Srn(2) * (x - S(0)); error_y *= error_y;
            double error_z = Srn(0) * (y - S(1)) - Srn(1) * (x - S(0)); error_z *= error_z;

            // NOTE! error is square error as sqrt(X)^2 = (\pm)X.
            error += error_x + error_y + error_z;

        }

        return error; 

    }


    Eigen::Vector3d computeIntersectionPoint(
                               const cv::Point2d &glint,
                               const std::vector<cv::Point3f> objectPoints,
                               const Eigen::Vector3d &eig_normal,
                               const Eigen::Matrix4d &transformation,
                               const Camera &cam) {

        // convert the 2D glint coordinates to a 3D direction vector
        cv::Point3d p3D;
        cam.pixToWorld(glint.x, glint.y, p3D);

        // check for nan
        if(p3D.x != p3D.x ||
           p3D.y != p3D.y ||
           p3D.z != p3D.z) {

            return Eigen::Vector3d();

        }

        // unit vector pointing towards the intersection
        Eigen::Vector3d eig_unitInters(p3D.x, p3D.y, p3D.z);


        size_t sz = objectPoints.size();
        std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > eig_transformed_object_points(sz);

        // put the transformed points into a vector of Eigen objects
        for(size_t c = 0; c < sz; ++c) {

            Eigen::Vector4d v((double)objectPoints[c].x,
                              (double)objectPoints[c].y,
                              (double)objectPoints[c].z,
                              1.0);

            Eigen::Vector4d res = transformation * v;

            Eigen::Vector3d &cur_point = eig_transformed_object_points[c];
            cur_point << res(0), res(1), res(2);

        }

        // find the intersection between the rectangle and the direction vector
        Eigen::Vector3d S;
        // return value is false only if no plane points were given
        /*bool bSuccess =*/ RectangleEstimator::findIntersection(
                                                     eig_unitInters,
                                                     eig_transformed_object_points,
                                                     eig_normal,
                                                     S);

        return S;

    }


    Eigen::Vector3d computeReflectionPoint(const Eigen::Vector3d &normal,
                                           const Eigen::Vector3d &S) {

        // compute the reflected direction vector
        Eigen::Vector3d Srn = RectangleEstimator::computeReflection(S, normal);

        return Srn;

    }


}	// end of namespace calib

