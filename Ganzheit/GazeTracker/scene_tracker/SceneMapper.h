#ifndef SCENEMAPPER_H
#define SCENEMAPPER_H

#include <Eigen/Core>
#include <Eigen/QR>
#include <Eigen/LU>
#include <Eigen/Geometry>

#include <opencv2/imgproc/imgproc.hpp>

#include "Camera.h"


class SceneMapper {

	public:

		SceneMapper() {}

		SceneMapper(Eigen::Matrix4d& A,
                    Camera* cam) {

			this->A = A;
			this->sceneCamera = cam;

		}

		void setMappingMatrix(Eigen::Matrix4d& A) {this->A = A;}

		/* Assign the pointer to the scene camera */
		void setCamera(Camera *_sceneCamera) {sceneCamera = sceneCamera;}

		/*
		 * Compute the 2D scene camera point, given the start and end points of
		 * the gaze vector in the eye camera coord. system.
		 */
		void getPosition(const Eigen::Vector3d& startPoint,
                         const Eigen::Vector3d& endPoint, cv::Point2d& point);


		bool kappaCorrection(Eigen::Vector3d& source,
                             Eigen::Vector3d& destination,
                             cv::Point2d& point); 

	private:

		void computeGazeVector(const Eigen::Vector3d &startPoint, // start point
							   const Eigen::Vector3d &endPoint,   // end point
							   Eigen::Vector3d &scenePoint,       // point in the scene camera coord. sys.
							   bool kappaCorrection);             // use kappa correction

		/* Transform between eye and scene camera coord. systems */
		Eigen::Matrix4d A;

		/* Scene camera parameters */
		Camera* sceneCamera;

};

#endif
