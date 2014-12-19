#include "jpeg.h"
#include <opencv2/highgui/highgui.hpp>
#include "VideoHandler.h"
#include "Saver.h"


static const int W_FRAME	= 640;
static const int H_FRAME	= 480;
static const int FRAMERATE	= 30;
static const int NDEVS		= 2;



int main() {

	gst_init(0, NULL);


	std::vector<VideoInfo> info(NDEVS);
	std::vector<std::string> save_paths(NDEVS);

	for(int i = 0; i < NDEVS; ++i) {

		char tmp[64];
		sprintf(tmp, "%s%d", "video", i+1);

		info[i].devname	= std::string(tmp);
		info[i].w		= W_FRAME;
		info[i].h		= H_FRAME;
		info[i].fps		= FRAMERATE;
		info[i].format	= FORMAT_MJPG;

		save_paths[i]	= std::string(tmp);
		save_paths[i].append(".mjpg");

	}


	Saver saver;
	if(!saver.init(save_paths)) {
		printf("Could not initialise saver\n");
		return -1;
	}

	VideoHandler *videos = new VideoHandler();
	if(!videos->init(info, &saver)) {
		std::cout << "Could not initialise the video handler" << std::endl;
		return -1;
	}


	// window names
	std::vector<std::string> win_names(NDEVS);
	for(int i = 0; i < NDEVS; ++i) {

		win_names[i] = info[i].devname;

	}


	while(1) {

		std::vector<CameraFrame *> imgs(NDEVS);

		// try get the video frames
		if(saver.getFrames(imgs)) {

			for(int i = 0; i < NDEVS; ++i) {

				CameraFrame *img = imgs[i];

				cv::Mat img_rgb(img->h,
								img->w,
								CV_8UC3,
								img->data,
								img->w * img->bpp);

				cv::Mat img_bgr;
				cv::cvtColor(img_rgb, img_bgr, CV_RGB2BGR);

				// draw to the GUI
				cv::imshow(win_names[i], img_bgr);

				// the caller of VideoHandler::getFrames() must release the frames
				delete img;

			}

		}


		// process keypress, exit if ESC
		int key = cv::waitKey(3);

		if((char)key == 27) {
			break;
		}

	}

	delete videos;


	pthread_exit(NULL);

}

