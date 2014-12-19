#ifndef LED_CALIB_SAMPLE_H
#define LED_CALIB_SAMPLE_H

#include "LEDCalibPattern.h"


namespace calib {


    class LEDCalibSample {

	public:

		LEDCalibSample() {

		}

		/* Copy contructor */
		LEDCalibSample(const LEDCalibSample &other) {

			image_points			= other.image_points;
			glint					= other.glint;
			img_path				= other.img_path;

		}


		/* Assignment operator */
		LEDCalibSample & operator= (const LEDCalibSample &other) {

			// protect against invalid self-assignment
			if(this != &other) {

				image_points			= other.image_points;
				glint					= other.glint;
				img_path				= other.img_path;

			}

			// by convention, always return *this
			return *this;

		}


		~LEDCalibSample() {

			image_points.clear();
			img_path.clear();

		}


		/* The locations of the found circles */
		std::vector<cv::Point2f> image_points;

		/* The location of the glint */
		cv::Point2d glint;

		/* The path to the image */
		std::string img_path;


        void clear() {

            image_points.clear();
            //		image_points.resize(16);
            img_path.clear();

            glint					= cv::Point2d(0.0, 0.0);

        }

    };



    /*
     * A container for LED-calibration-related data, i.e. samples and the result
     */
    class LEDCalibContainer {

	public:

		LEDCalibContainer() {

			const double _LED_pos[3] = {0.0, 0.0, 0.0};
			double _circleSpacing    = 0.0;
            const std::vector<LEDCalibSample> _samples;
            int _id = 0;

			init(_LED_pos,
				 _circleSpacing,
                 _samples,
                 _id);

		}


		/* Copy contructor */
		LEDCalibContainer(const LEDCalibContainer &other) {

            const double *_LED_pos                      = other.LED_pos;
            const double _circleSpacing                 = other.circleSpacing;
            const std::vector<LEDCalibSample> &_samples = other.samples;
            int _id                                     = other.id;

			init(_LED_pos,
				 _circleSpacing,
                 _samples,
                 _id);

		}


		/* Assignment operator */
		LEDCalibContainer & operator= (const LEDCalibContainer &other) {

			// protect against invalid self-assignment
			if(this != &other) {

				delete[] LED_pos;

				const double *_LED_pos                      = other.LED_pos;
				const double _circleSpacing                 = other.circleSpacing;
                const std::vector<LEDCalibSample> &_samples = other.samples;
                int _id                                     = other.id;

				init(_LED_pos,
					 _circleSpacing,
                     _samples,
                     _id);


			}

			// by convention, always return *this
			return *this;

		}


		~LEDCalibContainer() {

			delete[] LED_pos;

		}

		void addSample(const LEDCalibSample &_sample) {

			samples.push_back(_sample);

		}


		void addSample(const LEDCalibSample &_sample, const std::string &img_path) {

			samples.push_back(_sample);
			samples.back().img_path = img_path;

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


		std::vector<LEDCalibSample> &getSamples() {return samples;}

		const std::vector<LEDCalibSample> &getSamples() const {return samples;}


		bool isCalibrated() {

			bool b_allZero = (LED_pos[0] == 0.0) &&
                (LED_pos[1] == 0.0) &&
                (LED_pos[2] == 0.0);

			return !b_allZero;

		}


		void clear() {

			samples.clear();

			circleSpacing = 0.0;
			LED_pos[0] = LED_pos[1] = LED_pos[2] = 0.0;

		}


		/*
		 * The vectors towards the circles. The points are normalised, i.e.
		 * the circles spacing has not been taken into account.
		 */
		std::vector<cv::Point3f> normalisedObjectPoints;

		/* Circle spacing */
		double circleSpacing;

		/* The obtained LED position */
		double *LED_pos;

        /* LED id */
        int id;

	private:

		/* Called by the constructors */
		void init(const double _LED_pos[3],
				  double _circleSpacing,
                  const std::vector<LEDCalibSample> &_samples,
                  int _id) {

            LEDCalibPattern::getNormalisedLEDObjectPoints(normalisedObjectPoints);

			circleSpacing = _circleSpacing;
			LED_pos       = new double[3];
            samples       = _samples;
            id            = _id;

			memcpy(LED_pos, _LED_pos, 3 * sizeof(double));

		}

		/* LED calibration samples */
		std::vector<LEDCalibSample> samples;

    };




} // end of "namespace calib"


#endif
