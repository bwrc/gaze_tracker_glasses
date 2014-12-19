#include <cstdio>
#include <cmath>

#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/LU>
#include <Eigen/Geometry>

#include <opencv2/imgproc/imgproc.hpp>

#include "SceneMapper.h"
#include "Camera.h"
#include "trackerSettings.h"


void SceneMapper::getPosition(const Eigen::Vector3d& startPoint,
							  const Eigen::Vector3d& endPoint,
							  cv::Point2d& scenePoint) {

	// Compute the position in scene camera coordinates
	Eigen::Vector3d X;
	this->computeGazeVector(startPoint, endPoint, X, true);

	// Finally, compute the gaze coordinate in the image coordinates
	cv::Point3d worldPoint(X(0), X(1), X(2));
	double u, v;

	sceneCamera->worldToPix(worldPoint, &u, &v);
	scenePoint.x = u;
    scenePoint.y = v;

}


void SceneMapper::computeGazeVector(const Eigen::Vector3d &startPoint,
									const Eigen::Vector3d &endPoint,
									Eigen::Vector3d &point,
									bool kappaCorrect) {

	// Compute the direction vector
	Eigen::Vector3d direction = endPoint - startPoint;

	// Normalize the direction vector
	direction.normalize();

	// Compute the POG in eye camera coordinates
	const Eigen::Vector3d POG_ecam = direction * trackerSettings.GAZE_DISTANCE + startPoint;


	/*********************************************************************
	 * TODO: Kappa correction
	 ********************************************************************/

	if(kappaCorrect) {
		;
	}


	// Compute the position in scene camera coordinates
	Eigen::Vector4d tmp(POG_ecam(0), POG_ecam(1), POG_ecam(2), 1);
	Eigen::Vector4d X = A * tmp;

	point(0) = X(0);
	point(1) = X(1);
	point(2) = X(2);

}


bool SceneMapper::kappaCorrection(Eigen::Vector3d& source, Eigen::Vector3d& destination, cv::Point2d& point)
{

	// Determine the position of the cornea sphere in scene camera coordinates
	Eigen::Vector4d X0_tr(source(0), source(1), source(2), 1);
	X0_tr = this->A * X0_tr;
	Eigen::Vector3d X0(X0_tr(0), X0_tr(1), X0_tr(2));

	// Compute the direction vector from the camera
	cv::Point3d p3D;
	sceneCamera->pixToWorld(point.x, point.y, p3D);
	Eigen::Vector4d dX_tr(p3D.x, p3D.y, p3D.z, 1);
	Eigen::Vector3d dX(p3D.x, p3D.y, p3D.z);

	/*********************************************************************
	 * Determine the point where the direction vector and the cornea
	 * sphere intersects.
	 ********************************************************************/

    const double R = trackerSettings.GAZE_DISTANCE;

	double a = dX_tr(0) * dX_tr(0) + dX_tr(1) * dX_tr(1) + dX_tr(2) * dX_tr(2);
	double b = -2 * (dX_tr(0) * X0_tr(0) + dX_tr(1) * X0_tr(1) + dX_tr(2) * X0_tr(2));
	double c = X0_tr(0) * X0_tr(0) + X0_tr(1) * X0_tr(1) + X0_tr(2) * X0_tr(2) - R * R;

	const double discriminant = (b * b) - 4.0*a*c;
	if(discriminant < 0.0) {
		return false;
	}

	const double sqrt_discriminant = std::sqrt(discriminant);

	// the candidates for s
	double k1 = (-b + sqrt_discriminant) / (2.0*a);
	double k2 = (-b - sqrt_discriminant) / (2.0*a);

	double k = std::max(k1, k2);

	if(k < 0.0) {
		return false;
	}

	// Finally, retrieve the point of intersection
	Eigen::Vector3d POG = k * dX;

	/*********************************************************************
	 * TODO: Determine the angle between the optical vector and POG
	 ********************************************************************/


	return true;

}

