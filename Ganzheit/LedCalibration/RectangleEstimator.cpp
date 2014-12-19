#include "RectangleEstimator.h"
#include <gsl/gsl_multimin.h>

#include <string.h>
#include <stdio.h>



namespace RectangleEstimator {


    static double my_f(const gsl_vector *v, void *params);


    /*
     * Find the intersection between the vector and a plane defined by the
     * the points.
     */
    bool findIntersection(const Eigen::Vector3d &vec,
                          const std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > &planePoints,
                          const Eigen::Vector3d &normal,
                          Eigen::Vector3d &vecInters) {

        if(planePoints.size() == 0) {
            return false;
        }

        /*
         * Contains the plane points copied from "planePoints" plus
         * the normal and the direction vector appended to the end
         */
        std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > dataVec(planePoints.size() + 2);
        for(size_t i = 0; i < planePoints.size(); ++i) {
            dataVec[i] = planePoints[i];
        }
        dataVec[planePoints.size()]     = normal;
        dataVec[planePoints.size() + 1] = vec;


        /* Starting point */
        gsl_vector *x = gsl_vector_alloc(1);
        gsl_vector_set(x, 0, 1.0);

        /* Set initial step sizes to 1 */
        gsl_vector *ss = gsl_vector_alloc(1);
        gsl_vector_set_all(ss, 1.0);

        /* Initialize method and iterate */
        gsl_multimin_function minex_func;
        minex_func.n = 1;
        minex_func.f = &my_f;
        minex_func.params = (void *)&dataVec;

        const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex2;

        gsl_multimin_fminimizer *s = gsl_multimin_fminimizer_alloc(T, 1);
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
            status = gsl_multimin_test_size(size, 1e-7);

        } while(status == GSL_CONTINUE && iter < 1000);


        // length
        double d = gsl_vector_get(s->x, 0);

        vecInters(0) = vec(0) * d;
        vecInters(1) = vec(1) * d;
        vecInters(2) = vec(2) * d;

        gsl_vector_free(x);
        gsl_vector_free(ss);
        gsl_multimin_fminimizer_free(s);

        if(status != GSL_SUCCESS) {
            return false;
        }

        return true;

    }


    double my_f(const gsl_vector *v, void *params) {

        // get the plane points
        const std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > *planePoints = (std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > *)params;

        // plane points occupy the planePoints.size() - 2 first slots...
        int nPlanePoints = (int)planePoints->size() - 2;

        const Eigen::Vector3d &normal = planePoints->at(nPlanePoints);
        const Eigen::Vector3d &dirVec = planePoints->at(nPlanePoints + 1);

        // get the suggested distance
        const double distance = gsl_vector_get(v, 0);

        // compute the resulting vector with the suggested distance
        const Eigen::Vector3d resVec = distance * dirVec;

        /*************************************************************************
         * Loop through the points and compute the error
         *************************************************************************/

        double errTot = 0.0;

        for(size_t i = 0; i < nPlanePoints; ++i) {

            const Eigen::Vector3d &Si = planePoints->at(i);

            /*
             * The vector intersects the plane when dot((d*Vn-Si), n) = 0,
             * where:
             *     Vn: given direction vector
             *     V:  V = d * Vn
             *     d:  length of V
             *     Si: the current plane point
             *     n:  plane normal
             */

            double err = normal.dot(resVec - Si);
            err *= err;
            errTot += err;

        }

        return errTot;

    }


    /*
     *	Computes the reflection of the given intersecting vector
     *
     *	http://en.wikipedia.org/wiki/Snell's_law#Vector_form
     *	cos(ang) = dot(n, -v)
     *	v_reflect = v + (2*cos(ang))*n
     */
    Eigen::Vector3d computeReflection(const Eigen::Vector3d &vr, const Eigen::Vector3d &normal) {

        // normalise the input vector
        Eigen::Vector3d vrn = vr.normalized();

        // no need to divide by the product of the lengths because the vectors are already normalised
        double cos_theta1 = normal.dot(-vrn);

        Eigen::Vector3d v_reflect = vrn + (2.0*cos_theta1) * normal;
        v_reflect.normalize();

        return v_reflect;

    }


} // end of "namespace RectangleEstimator {"

