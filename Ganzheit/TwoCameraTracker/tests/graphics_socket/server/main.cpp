#include "Server.h"
#include <stdio.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>


static const int IMG_W		= 640;
static const int IMG_H		= 480;
static const int BUFF_SZ	= IMG_W * IMG_H * 3;


int main(int nargs, char *args[]) {

	if(nargs != 2) {

		printf("Give the sun path\n");

		return -1;

	}


	gtSocket::Server server;
	if(!server.init(args[1], BUFF_SZ)) {

		printf("%s\n", server.getError().c_str());

		return -1;

	}

	printf("Establish connection...");
	fflush(stdout);

	if(!server.start()) {

		printf("%s\n", server.getError().c_str());

		return -1;

	}

	printf("ok\n");


	std::string strWin("server");
	while(1) {

		int nRead = server.receive(BUFF_SZ, true);
		if(nRead != BUFF_SZ) {
			printf("Got %d requested %d\n", nRead, BUFF_SZ);

			continue;
		}


		const char *buff = server.getBuffer();

		// image header
		cv::Mat imgBGR(
			IMG_H,
			IMG_W,
			CV_8UC3,
			(void *)buff
		);

		cv::imshow(strWin, imgBGR);

		cv::waitKey(1);

	}


	return 0;

}

