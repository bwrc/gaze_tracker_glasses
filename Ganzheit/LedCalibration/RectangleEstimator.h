#ifndef RECTANGLEESTIMATOR_H
#define RECTANGLEESTIMATOR_H


#include <string.h>
#include <Eigen/Core>
#include <vector>


namespace RectangleEstimator {


	/*
	 * Find the intersection between a vector and a plane defined by the
	 * the points.
	 */
	bool findIntersection(const Eigen::Vector3d &vec,
                          const std::vector<Eigen::Vector3d, Eigen::aligned_allocator<Eigen::Vector3d> > &_plane_points,
                              const Eigen::Vector3d &normal,
                              Eigen::Vector3d &vecInters);


	/*
	 *	Computes the reflection of the given intersecting vector
	 *
	 *	http://en.wikipedia.org/wiki/Snell's_law#Vector_form
	 *	cos(ang) = dot(n, -v)
	 *	v_reflect = v + (2*cos(ang))*n
	 */
	Eigen::Vector3d computeReflection(const Eigen::Vector3d &vrn, const Eigen::Vector3d &normal);

}


#endif

