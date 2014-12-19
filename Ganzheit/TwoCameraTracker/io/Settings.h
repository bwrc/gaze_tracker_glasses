#ifndef SETTINGS_H
#define SETTINGS_H


#include <string>
#include <tinyxml.h>


/*
 *	Example:
 *
 *	<document>
 *
 *		<settings id="output">
 *			<directory value="somedir" />
 *		</settings>
 *
 *		<settings id="input_devices">
 *			<dev1 value="video1" />
 *			<dev2 value="video2" />
 *		</settings>
 *
 *		<settings id="input_files">
 *			<gazetracker value = "default.xml" />
 *			<eyeCamCalibration value = "eyeCam.calib" />
 *			<sceneCamCalibration value = "sceneCam.calib" />
 *			<mapper value = "mapper.yaml" />
 *		</settings>
 *
 *	</document>
 */
class Settings {

	public:

		/* Read settings from the given file */
		bool readSettings(const char *fname);


		/* Cameras 1 and 2 */
		std::string dev1;
		std::string dev2;


		/*
		 * Directory for the output files:
		 *   1. camera 1
		 *   2. camera 2
		 *   3. results
		 */
		std::string oput_dir;


		/* Gaze tracker settings file */
		std::string gazetrackerFile;

		/* The results of the camera and LED calibrations */
		std::string eyeCamCalibFile;

		/* The results of the scene camera calibration */
		std::string sceneCamCalibFile;

		/* The transformation matrix between the eye and the scene cameras */
		std::string mapperFile;

	private:

		/* Get a string corresponding the attribute and parameter */
		std::string getString(TiXmlElement *rootElement, const char *attr, const char *param);

};



#endif

