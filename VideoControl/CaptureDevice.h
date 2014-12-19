/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <arto.merilainen@ttl.fi> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 */

#ifndef CAPTUREDEVICE_H
#define CAPTUREDEVICE_H


#include "gst/gst.h"

#include "CameraFrame.h"


/*
 * An instance of this class should be given to the VideoHandler.
 * The CaptureDevice calls frameReceived() when a frame is received.
 * 
 */
class FrameReceiver {

	public:


		/*
		 * Must be mutex protected by the user when multiple streams are
		 * being used.
		 */
		virtual void frameReceived(const CameraFrame *frame, int id) = 0;

		virtual ~FrameReceiver() {}
};


/* Forward declare the CaptureDevice */
class CaptureDevice;


/*
 * A container for the callbackdata given to gstreamer.1
 */
class CBData {

	public:

		CBData() {
			cap_dev	= NULL;
		}

		CaptureDevice *cap_dev;

		VideoInfo info;

};



/*
 * This class is the front-end for gstreamer. It is used for creating a camera
 * stream and, in the future, a video file stream.
 */
class CaptureDevice {

	public:

		CaptureDevice();

		~CaptureDevice();

		/***************************************************************
		 * Open a video device. The device is automatically started
		 * to stream data from the camera.
		 **************************************************************/

		bool init(const std::string &devpath,
				  const int width,
				  const int height,
				  const int framerate,
				  const Format format,
				  FrameReceiver *receiver,
				  int id);

		/* The frame receiver, defined by the user */
		FrameReceiver *receiver;

		/* Get the id of this device, also user defined */
		int getID() {return id;}

	private:

		GstElement *pipeline;
		void deletePipeline();

		/* Video information, i.e. dimensions, paths etc. */
		VideoInfo video_info;


		/* callback data */
		CBData cbdata;

		/* User specified ID */
		int id;
};


#endif // CAPTUREDEVICE_H

