#ifndef VIDEOHANDLER_H
#define VIDEOHANDLER_H


/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <sharman.jagadeesan@ttl.fi> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return Poul-Henning Kamp
 * ----------------------------------------------------------------------------
 */



#include <opencv2/imgproc/imgproc.hpp>
#include "CaptureDevice.h"
#include "VideoBuffer.h"
#include "JPEGWorker.h"
#include <vector>
#include <fstream>




/*
 * This class initialises video streaming. The number of streams can
 * be anything from 1 to N, as long as the USB bus is capable of handling
 * the data.
 *
 * This class initialises the streams which starts the reveiving of frames
 * from gstreamer. The user-defined sub-class of FrameReceiver will
 * receive the frames.
 *
 */
class VideoHandler {

	public:

		~VideoHandler();

		/*
		 * Start the stream on the defined devices. true or false
		 * upon success and failure respectively
		 */
		bool init(const std::vector<VideoInfo> &info, FrameReceiver *_r);


		/* Get the number of streams handled by this class */
		size_t getNofStreams() {return capdevices.size();}

	private:

		/* Create the capture devices using std::vector<VideoIngo> info */
		bool createCaptureDevices(const std::vector<VideoInfo> &info);

		/* A vector of device names. Should be of the form "videoX" */
		std::vector<std::string> devnames;

		/* Individual stream pipelines */
		std::vector<CaptureDevice *> capdevices;

		/* Stores the video informatio for each stream */
		std::vector<VideoInfo> video_info;

		/* A pointer to the user-defined frame receiver */
		FrameReceiver *frameReceiver;

};


#endif

