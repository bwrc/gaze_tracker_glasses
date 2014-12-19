#include "SimpleCapture.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <fstream>


static const int W_FRAME	= 640;
static const int H_FRAME	= 480;
static const int FRAMERATE	= 30;


bool createStream(SimpleCapture &device,
				   std::string &devname) {

	if(!device.open(devname,			// device path
					W_FRAME,			// witdth
					H_FRAME,			// height
					FRAMERATE,			// framerate
					FORMAT_RGB)) {		// format

		printf("Could not initialise the capture device: %s\n", devname.c_str());

		return false;

	}


	return true;

}


int main(int nargs, char **args) {

	if(nargs != 2) {
		printf("Usage:\n    ./cam <ID[0, 1, 2, ...]>\n");
		return EXIT_FAILURE;
	}

	// initialise gst. Exits the program on failure.
	gst_init(0, NULL);


	/***********************************************************************
	 * create streams
	 ***********************************************************************/
	const char *video_dev = args[1];
	SimpleCapture simple_cap;
	std::string devname = std::string("/dev/video");
	devname.append(video_dev);


	if(!createStream(simple_cap, devname)) {

		printf("Could not create stream\n");

		return EXIT_FAILURE;
	}



	/***********************************************************************
	 * create window
	 ***********************************************************************/
	std::string win_name("win");
	cv::namedWindow(win_name, CV_WINDOW_AUTOSIZE);


	/***********************************************************************
	 * loop
	 ***********************************************************************/

	while(1) {

		/***********************************************************************
		 * get a frame
		 ***********************************************************************/
		CameraFrame *frame = simple_cap.grabFrame(true);
		if(frame == NULL) {

			printf("grab failed\n");

			return EXIT_FAILURE;

		}


		/***********************************************************************
		 * convert the data to BGR and draw
		 ***********************************************************************/
		cv::Mat img_rgb(frame->h,
						frame->w,
						CV_8UC3,
						(void*)frame->data,
						frame->w * frame->bpp);

		cv::Mat img_bgr;

		cv::cvtColor(img_rgb, img_bgr, CV_RGB2BGR);

        delete frame;

		// show the image
		imshow(win_name, img_bgr);

		// get a keypress
		int key = cv::waitKey(3);

		// exit if ESC
		if((char)key == 27) {
			break;
		}

	}


	return EXIT_SUCCESS;

}

