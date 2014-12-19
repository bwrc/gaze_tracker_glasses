#include <cmath>

#include <fcntl.h>              /* low-level i/o */
#include <sys/ioctl.h>

#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>
#include <unistd.h>

#include "VideoControl.h"

VideoControl::VideoControl(std::string device)
{
	this->fd = -1;
	this->open_dev(device);
}

VideoControl::VideoControl()
{
	this->fd = -1;
}

VideoControl::~VideoControl()
{
	this->close_dev();
}

bool VideoControl::open_dev(std::string device)
{
	this->fd = open(device.c_str(), O_RDWR | O_NONBLOCK, 0);
	return this->isOpen();
}

bool VideoControl::isOpen()
{
	return fd == -1 ? false : true;
}

void VideoControl::close_dev()
{
	if(this->isOpen()) {
		close(this->fd);
	}
}

bool VideoControl::setFocus(bool automatic, double value)
{
	// Set auto/manual for the focus
	if(!this->setSingle(V4L2_CID_FOCUS_AUTO, automatic ? 1 : 0)) {
		return false;
	}

	// Set focus value
	if(!this->setScaled(V4L2_CID_FOCUS_ABSOLUTE, value)) {
		return false;
	}

	return true;
}

bool VideoControl::setExposure(ExposureType exposure_type, int value)
{
	// Set exposure type
	if(!this->setSingle(V4L2_CID_EXPOSURE_AUTO, exposure_type)) {
		return false;
	}

	// No need to set the absolute value...
	if(exposure_type != EXPOSURE_MANUAL) {
		return true;
	}

	// Set the exposure value (NOTE!! No scaling here)
	if(!this->setSingle(V4L2_CID_EXPOSURE_ABSOLUTE, value)) {
		return false;
	}

	return true;
}

bool VideoControl::setZoom(ZoomType setting, double value)
{
	return setScaled(setting, value);
}

bool VideoControl::setTilt(double value)
{
	return setScaled(V4L2_CID_TILT_ABSOLUTE, value);
}

bool VideoControl::setPan(double value)
{
	return setScaled(V4L2_CID_PAN_ABSOLUTE, value);
}

bool VideoControl::setScaled(int setting, double value)
{
	// Get minimum and maximum values
	int min, max;
	if(this->getSettings(setting, max, min)) {
		return false;
	}

	// Scale the given value
	int scaledVal = std::floor((double)min + (double)(max - min) * value + 0.5);

	// Set the value
	return setSingle(setting, scaledVal);

}

bool VideoControl::setSingle(int setting, int value)
{
	struct v4l2_ext_control ectrl[1];
	struct v4l2_ext_controls ectrls;

	ectrls.count = 1;
	ectrls.controls = ectrl;
	ectrls.ctrl_class = V4L2_CTRL_CLASS_USER;

	ectrl[0].id = setting;
	ectrl[0].value = value;

	int ret = ioctl(fd, VIDIOC_S_EXT_CTRLS, &ectrls);

	if(!ret)
		return true;

	return false;
}

int VideoControl::getSettings(int setting, int &max, int &min)
{
	struct v4l2_queryctrl ctrl;

	ctrl.id = setting;
	int ret = ioctl(fd, VIDIOC_QUERYCTRL, &ctrl);
	min = ctrl.minimum;
	max = ctrl.maximum;

	return ret;
}

