#ifndef SCENE_TRACKER_H
#define SCENE_TRACKER_H

#include <vector>
#include <opencv2/imgproc/imgproc.hpp>

class CalibrationPoint {
	public:

		CalibrationPoint(const cv::Point2f& eyePoint,
				const cv::Point2f& glintPoint,
				const cv::Point2f& scenePoint) {

			this->eyePoint = eyePoint;
			this->glintPoint = glintPoint;
			this->scenePoint = scenePoint;
		}

		cv::Point2f eyePoint;
		cv::Point2f glintPoint;
		cv::Point2f scenePoint;
};

class SceneTracker {
	public:
		SceneTracker() {
			calibrationComplete = false;
		}

		bool calibrate(const std::vector<CalibrationPoint>& points);
		cv::Point2f track(const cv::Point2f& eyePoint, const cv::Point2f& glintPoint);

	private:

		class CalibrationVector {
			public:

				CalibrationVector(const CalibrationPoint & point) {

					this->eyeVector = point.eyePoint - point.glintPoint;
					this->scenePoint = point.scenePoint;
				}

				CalibrationVector(const cv::Point2f& eyeVector,
						const cv::Point2f& scenePoint) {

					this->eyeVector = eyeVector;
					this->scenePoint = scenePoint;
				}

				cv::Point2f eyeVector;
				cv::Point2f scenePoint;
		};

		void cal_calibration_homography(const std::vector<CalibrationVector>& calibrationPoints);
		void normalize_point_set(const std::vector<CalibrationVector>& point_set,
					std::vector<CalibrationVector>& norm_point_set,
					double &dis_scale_eye, double& dis_scale_scene, cv::Point2f& nor_center_eye,
					cv::Point2f& nor_center_scene);
		void affine_matrix_inverse(double a[][3], double r[][3]);
		void matrix_multiply33(double a[][3], double b[][3], double r[][3]);

		double map_matrix[3][3];
		bool calibrationComplete;
};

#endif
