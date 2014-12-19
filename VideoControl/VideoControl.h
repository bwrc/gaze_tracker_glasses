#ifndef VIDEO_CONTROL_H
#define VIDEOCONTROL_H

#include <iostream>
#include <linux/videodev2.h>

// V4L2 has three different zoom types. USe ZOOM_ABSOLUTE if you're uncertain
enum ZoomType {
	ZOOM_ABSOLUTE = V4L2_CID_ZOOM_ABSOLUTE,
	ZOOM_RELATIVE = V4L2_CID_ZOOM_RELATIVE,
	ZOOM_CONTINUOUS = V4L2_CID_ZOOM_CONTINUOUS
};

enum ExposureType {
	EXPOSURE_AUTO = V4L2_EXPOSURE_AUTO,
	EXPOSURE_MANUAL = V4L2_EXPOSURE_MANUAL,
	EXPOSURE_SHUTTER_PRIORITY = V4L2_EXPOSURE_SHUTTER_PRIORITY,
	EXPOSURE_APERTURE_PRIORITY = V4L2_EXPOSURE_APERTURE_PRIORITY
};

class VideoControl {
	public:
		VideoControl(std::string device);
		VideoControl();
		~VideoControl();

		bool open_dev(std::string device);
		bool isOpen();
		void close_dev();

		/* Basic interface to control the camera. Value is a double
		 * between 0 and 1 */

		bool setFocus(bool automatic, double value);
		bool setZoom(ZoomType setting, double value);
		bool setTilt(double value);
		bool setPan(double value);

		/* NOTE!! MS cam takes the exposure in some very fancy format..
		 * In other words, it is not linear, but something weird. Thus,
		 * the value cannot be scaled! (if unsure, use 625) */

		bool setExposure(ExposureType exposure_type, int value);
	private:

		int getSettings(int setting, int &max, int &min);
		bool setSingle(int setting, int value);
		bool setScaled(int setting, double value);
		int fd;
};

#endif
