
#ifndef GAZETRACKER_H
#define GAZETRACKER_H


#include "PupilTracker.h"
#include "Cornea_computer.h"
#include "Camera.h"
#include <vector>



namespace gt {

    /*
     * This is a container to hold information about:
     *
     *     1. The calibrated LED position
     *     2. Direction vector towards the glint
     *     3. The glint in the image
     *     4. The label to tell which LED this is in the six-LED configuration
     */
    class LED {
	public:

        /*
         * Constructor
         */
		LED();

        /*
         * Accessors
         */
        const cv::Point3d &pos() const;
		const cv::Point3d *getGlintDirection3D() const;
        const cv::Point2d &getGlint2D() const {return m_posGlint2D;}
        int getLabel() const {return m_nLabel;}

        /*
         * Set parameters
         */
		void setPos(const cv::Point3d &pos) {m_posLED3D = pos;}
		void setGlintDirection3D(const cv::Point3d &pos) {m_directionGlint3D = pos;}
		void setGlint2D(const cv::Point2d &pos) {m_posGlint2D = pos;}
        void setLabel(int nLabel) {m_nLabel = nLabel;}

	private:

        /*
         * Physical position of the LED in the eye camera coordinate system
         */
        cv::Point3d m_posLED3D;

        /*
         * Direction vector towards the glint
         */
        cv::Point3d m_directionGlint3D;

        /*
         * Position of the glint, obtained from PupilTracker
         */
        cv::Point2d m_posGlint2D;

        /*
         * Label to map the glint to the LED int the six-LED configuration.
         * This is -1 when unidentified.
         */
        int m_nLabel;

    };


    class GazeTracker {

    private:

        /*
         * An interface for a LED configuration.
         */
        class LEDPattern {

        public:

            /*
             * Vector of positions of the LEDs in the eye camera coord system
             */
            LEDPattern(const std::vector<cv::Point3d> &positions) {};

            /*
             * The destructor must always be defined in base classes.
             */
            virtual ~LEDPattern() {}

            /*
             * This function should set the leds vector.
             *
             * glints:   2D locations of the corneal reflections
             * pc:       pupil centre
             * glints3d: corresponding 3D direction vectors in the same order
             */
            virtual bool associateGlints(const std::vector<cv::Point2d> &glints,
                                         cv::Point2f pc,
                                         const std::vector<cv::Point3d> &glints3d) = 0;

            /*
             * Compute the cornea centre
             */
            virtual void getCorneaCentre(Cornea *cornea, cv::Point3d &centreCornea) = 0;

            /*
             * Access the LEDs
             */
            const std::vector<LED> &getLEDs() {return leds;}

        protected:

            /*
             * Vector of LEDs.
             */
            std::vector<LED> leds;

        };


        /*
         * This class inherits from LEDPattern. The class represents the following
         * LED configuration.
         *
         * The six-glint pattern
         *
         *              0
         *              o
         *
         *      1 o           o 5
         *
         *      2 o           o 4
         *
         *              o
         *              3
         *
         */
        class SixLEDs : public LEDPattern {

        public:

            SixLEDs(const std::vector<cv::Point3d> &positions);

            bool associateGlints(const std::vector<cv::Point2d> &glints,
                                 cv::Point2f pc,
                                 const std::vector<cv::Point3d> &glints_3d);

            void getCorneaCentre(Cornea *cornea, cv::Point3d &centreCornea);

        };


	public:

        /*
         * Constructor.
         */
		GazeTracker();
		~GazeTracker();

        /*
         * Must be called before starting to track. Can be called multiple times.
         */
		void init(const Camera &camera,
                  const std::vector<cv::Point3d> &vecLEDPositions);

		/*
         * Track the gaze
         */
		bool track(const cv::Mat &img);

        /*
         * Accessors
         */
		PupilTracker *           getPupilTracker()         {return pupil_tracker;}
		const PupilTracker *     getPupilTracker()  const  {return pupil_tracker;}
		Cornea *                 getCorneaComputer()       {return cornea;}
		const double *           getCentrePupil()    const {return centre_pupil;}
		const double *           getCentreCornea()   const {return centre_cornea;}
		const Camera &           getCamera()         const {return m_camera;}
		const std::vector<LED> & getLEDs()           const {return m_pPattern->getLEDs();}

        /*
         * Reset the parameters, called at the beginning of track().
         */
		void reset();


		/* Get the duration in microseconds of the last track */
		long getTrackDurationMicros() {return track_dur_micros;}

        /*
         * Get pupil radius
         */
        double getPupilRadius() {return m_dPupilRadius;}

	private:

        /*
         * Compute the pupil radius from m_vec3DPerimeterPoints. The pupil centre
         * centre_pupil must be known before calling this function.
         */
        void computePupilRadius();

        /*
         * Get the ellipse perimeter points. The angles are defined in the
         * ellipse's coordinate system.
         */
		static void getEllipsePoints(const cv::RotatedRect &ellipse,
                                     double ang_p_start,
                                     double ang_p_end,
                                     std::vector<cv::Point2d> &points);


        /*
         * Same as the above, but the start and end angles are 0 and 2PI respectively.
         */
		static void getEllipsePoints(const cv::RotatedRect &ellipse,
                                     std::vector<cv::Point2d> &points);

        /*
         * Track the direction vector to the pupil perimeter.
         */
		bool traceDirVecToPupil(const cv::Point3d &dir_vec,	// the direction vector
                                const cv::Point3d &cw,		// the cornea centre
                                double rps2,					// the squared distance from cw to the pupil perimeter point
                                cv::Point3d &pupil_point);	// the resulting point on the pupil perimeter


		double getRP(const cv::Point3d &cw);

		PupilTracker *pupil_tracker;
		Cornea *cornea;
		Camera m_camera;

		double centre_pupil[3];
		double centre_cornea[3];

		LEDPattern *m_pPattern;

		/* Duration it took for the last track */
		long track_dur_micros;

        /*
         * Ray traced 3D pupil perimeter points
         */
        std::list<cv::Point3d> m_vec3DPerimeterPoints;

        /*
         * Estimated pupil radius
         */
        double m_dPupilRadius;

    };


} // end of namespace gt {



#endif

