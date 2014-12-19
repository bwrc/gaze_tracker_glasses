#ifndef CAMERA_CALIB_SAMPLE_H
#define CAMERA_CALIB_SAMPLE_H

#include "Camera.h"
#include <opencv2/core/core.hpp>
#include <vector>
#include <list>


namespace calib {

    class CameraCalibSample {

	public:

		/* Copy contructor */
		CameraCalibSample() {

			image_points.resize(44);

		}

		/* Copy contructor */
		CameraCalibSample(const CameraCalibSample &other) {
			img_path = other.img_path;
			image_points = other.image_points;
		}

		void clear() {

			img_path.clear();
			image_points.clear();
			image_points.resize(44);

		}

		/* The location of found circles */
		std::vector<cv::Point2f> image_points;

		/* Path to the image */
		std::string img_path;

    };



    /*
     * A container for camera-calibration-related data, i.e. samples and the results
     */
    class CameraCalibContainer {

	public:

		CameraCalibContainer() {

			double _intr[9];
			double _dist[5];
			memset(_intr, 0, 9*sizeof(double));
			memset(_dist, 0, 5*sizeof(double));

			std::vector<CameraCalibSample> _samples;
			std::vector<cv::Point3f> _object_points;
			double _reproj_err = 0.0;
			cv::Size _imgSize(0, 0);

			init(_samples,
				 _object_points,
				 _intr,
				 _dist,
				 _reproj_err,
				 _imgSize);

		}


		CameraCalibContainer(const std::vector<cv::Point3f> &_object_points, const cv::Size &_imgSize) {

			double _intr[9];
			double _dist[5];
			memset(_intr, 0, 9*sizeof(double));
			memset(_dist, 0, 5*sizeof(double));

			std::vector<CameraCalibSample> _samples;
			double _reproj_err = 0.0;

			init(_samples,
				 _object_points,
				 _intr,
				 _dist,
				 _reproj_err,
				 _imgSize);

		}


		/* Copy contructor */
		CameraCalibContainer(const CameraCalibContainer &other) {

			const std::vector<cv::Point3f> &_object_points	= other.object_points;
			const double *_intr								= other.intr;
			const double *_dist								= other.dist;
			const double _reproj_err						= other.reproj_err;
			const std::vector<CameraCalibSample> &_samples	= other.samples;
			const cv::Size _imgSize							= other.imgSize;

			init(_samples,
				 _object_points,
				 _intr,
				 _dist,
				 _reproj_err,
				 _imgSize);

		}


		/* Assignment operator */
		CameraCalibContainer & operator= (const CameraCalibContainer &other) {

			// protect against invalid self-assignment
			if(this != &other) {

				delete[] intr;
				delete[] dist;

				const std::vector<cv::Point3f> &_object_points	= other.object_points;
				const double *_intr								= other.intr;
				const double *_dist								= other.dist;
				const double _reproj_err						= other.reproj_err;
				const std::vector<CameraCalibSample> &_samples	= other.samples;
				const cv::Size _imgSize							= other.imgSize;

				init(_samples,
					 _object_points,
					 _intr,
					 _dist,
					 _reproj_err,
					 _imgSize);

			}

			// by convention, always return *this
			return *this;

		}


		~CameraCalibContainer() {

			delete[] intr;
			delete[] dist;

		}


		/* Construct a camera object intrinsic   */
		Camera makeCameraObject() const {

			Camera cam;
			cam.setIntrinsicMatrix(intr);
			cam.setDistortion(dist);

			return cam;

		}


		std::vector<CameraCalibSample> &getSamples() {return samples;}

		const std::vector<CameraCalibSample> &getSamples() const {return samples;}


		bool isCalibrated() {

			std::vector<double> _intr(9, 0.0);
			if(memcmp(intr, &_intr[0], 9*sizeof(double)) == 0) {

				std::vector<double> _dist(5, 0.0);
				if(memcmp(dist, &_dist[0], 5*sizeof(double)) == 0) {

					if(reproj_err == 0.0) {

						// all are zero
						return false;

					}

				}

			}

			return true;

		}


		void clear() {

			// clear the samples
			samples.clear();

			// TODO: check if this should be called
            //			object_points.clear();

			//imgSize = cv::Size(0, 0);
			memset(intr, 0, 9*sizeof(double));
			memset(dist, 0, 5*sizeof(double));
			reproj_err = 0.0;

		}

		void addSample(const CameraCalibSample &_sample) {
			samples.push_back(_sample);
		}


		void addSample(const CameraCalibSample &_sample, const std::string &_img_path) {

			samples.push_back(_sample);
			CameraCalibSample &last = samples.back();
			last.img_path = _img_path;

		}


		void deleteSample(int index) {

			samples.erase(samples.begin() + index);

		}


		/* */
		void deleteSamples(std::list<int> &indices) {

			// ascending sort
			indices.sort();

			std::list<int>::const_reverse_iterator rit = indices.rbegin();


			while(rit != indices.rend()) {

				samples.erase(samples.begin() + *rit);

				++rit;

			}

		}


		/* Object points */
		std::vector<cv::Point3f> object_points;

		/* The obtained intrinsic matrix. Column-major order. */
		double *intr;

		/* The obtained distortion coefficient vector */
		double *dist;

		/* The obtained reprojection error */
		double reproj_err;

		/* The size of the image */
		cv::Size imgSize;

	private:

		void init(const std::vector<CameraCalibSample> &_samples,
				  const std::vector<cv::Point3f> &_object_points,
				  const double *_intr,
				  const double *_dist, 
				  double _reproj_err,
				  const cv::Size &_imgSize) {

			samples			= _samples;
			object_points	= _object_points;
			reproj_err		= _reproj_err;
			imgSize			= _imgSize;

			intr = new double[9];
			dist = new double[5];

			memcpy(intr, _intr, 9*sizeof(double));
			memcpy(dist, _dist, 5*sizeof(double));

		}


		/* Samples used in calibrating the camera parameters */
		std::vector<CameraCalibSample> samples;

    };


} // end of "namespace calib"


#endif

