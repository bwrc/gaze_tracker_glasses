#include "VideoHandler.h"
#include <list>
#include <gst/gst.h>
#include <gst/video/video.h>


VideoHandler::~VideoHandler() {

	// destroy the capture devices
	for(size_t i = 0; i < capdevices.size(); ++i) {

		CaptureDevice *curdevice = capdevices[i];

		delete curdevice;
		capdevices[i] = NULL;

	}

}


bool VideoHandler::init(const std::vector<VideoInfo> &_info, FrameReceiver *_r) {

	// copy the frameReceiver
	frameReceiver = _r;

	// copy video information
	video_info = _info;

	// create the capture devices
	if(!createCaptureDevices(video_info)) {
		return false;
	}


	return true;

}


bool VideoHandler::createCaptureDevices(const std::vector<VideoInfo> &info) {

	// get the number of devices
	const size_t ndevs = info.size();


	// allocate space for the capture device pointers
	capdevices.resize(ndevs);

	for(size_t i = 0; i < ndevs; ++i) {

		// extract stream info
		const int frame_w	= info[i].w;
		const int frame_h	= info[i].h;
		const int fps		= info[i].fps;
		const Format format	= info[i].format;

		// create the capture device
		CaptureDevice *newdev = new CaptureDevice();

		// place it into the list
		capdevices[i] = newdev;


		// initialise the capture device
		if(!newdev->init(info[i].devname,   // device path
						 frame_w,			// width
						 frame_h,			// height
						 fps,				// framerate
						 format,			// format
						 frameReceiver,		// frame receiver
						 i)) {				// id of this device

			printf("Could not initialise the capture device: %s\n",
					info[i].devname.c_str());

			return false;

		}

	}

	return true;

}

