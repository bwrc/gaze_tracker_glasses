#include "CaptureDevice.h"
#include "jpeg.h"
#include <opencv2/highgui/highgui.hpp>
#include <fstream>


static const int W_FRAME	= 640;
static const int H_FRAME	= 480;
static const int FRAMERATE	= 30;
static const int COLOR_RED = 2;
static const int COLOR_GREEN = 1;


void remove_channel(cv::Mat &img_bgr, int c) {

	int w = img_bgr.cols;
	int h = img_bgr.rows;
	unsigned char *data = img_bgr.data;
	int step = img_bgr.step;


	for(int row = 0; row < h; ++row) {

		for(int col = 0; col < w; ++col) {

			int ind = 3*(row * w + col);

			data[ind+c] = 0;

		}

	}


}


bool createStreams(std::vector<CaptureDevice> &devices,
				   std::vector<std::string> &devnames) {

	size_t sz = devices.size();

	if(sz != devnames.size()) {
		return false;
	}

	for(size_t i = 0; i < sz; ++i) {

		CaptureDevice &curdev = devices[i];

		std::string devpath("/dev/");
		devpath.append(devnames[i]);

		if(!curdev.init(devpath,			// device path
						W_FRAME,			// witdth
						H_FRAME,			// height
						FRAMERATE,			// framerate
						FORMAT_MJPG)) {		// format

			std::cout << "Could not initialise the capture device: " <<  devnames[i] << std::endl;

			return false;

		}

	}

	return true;

}


int main() {

	gst_init(0, NULL);

	/***********************************************************************
	 * create streams
	 ***********************************************************************/

	std::vector<CaptureDevice> caps(2);
	std::vector<std::string> devnames(2);
	devnames[0] = std::string("video1");
	devnames[1] = std::string("video2");


	if(!createStreams(caps, devnames)) {

		printf("Could not create streams\n");

		return EXIT_FAILURE;
	}



	/***********************************************************************
	 * create windows
	 ***********************************************************************/
	size_t nstreams = devnames.size();
	std::vector<std::string> win_names(nstreams);

	for(size_t i = 0; i < nstreams; ++i) {

		win_names[i] = devnames[i];

		cv::namedWindow(win_names[i], CV_WINDOW_AUTOSIZE);
	}




	/***********************************************************************
	 * loop
	 ***********************************************************************/
	cv::Mat img_bgr(480, 640, CV_8UC3);



cv::Mat sumimg(480, 640, CV_8UC3);


	while(1) {

memset(sumimg.data, 0, 3*640*480);

		for(size_t i = 0; i < nstreams; ++i) {

			/***********************************************************************
			 * get a frame
			 ***********************************************************************/
			CameraFrame *frame = caps[i].getCameraFrame();
			if(frame == NULL) {
				std::cout << "grabbed a NULL frame from: " << devnames[i] << std::endl;
				return EXIT_FAILURE;
			}


			/***********************************************************************
			 * decompress the frame
			 ***********************************************************************/
			JPEG_Decompressor jpgd;
			bool success = jpgd.decompress(frame->data, frame->sz);
			if(!success) {
				std::cout << "Could not decompress a frame from: " << devnames[i] << std::endl;
				return EXIT_FAILURE;
			}



			/***********************************************************************
			 * convert the data to BGR and draw
			 ***********************************************************************/
			size_t sz;
			const unsigned char *raw_data = jpgd.getRawData(sz);

			cv::Mat img_rgb(frame->h,
							frame->w,
							CV_8UC3,
							(void*)raw_data,
							frame->w*frame->bpp);

			cv::cvtColor(img_rgb, img_bgr, CV_RGB2BGR);

			imshow(win_names[i], img_bgr);

// remove red or green
remove_channel(img_bgr, i == 1 ? COLOR_RED : COLOR_GREEN);
sumimg += 0.5*img_bgr;

			delete frame;

		}


cv::imshow("jee", sumimg);

		int key = cv::waitKey(3);

		if((char)key == 27) {
			break;
		}

	}


	return EXIT_SUCCESS;

}

