#include "Client.h"
#include <stdio.h>
#include <opencv2/imgproc/imgproc.hpp>


static const int IMG_W		= 640;
static const int IMG_H		= 480;
static const int BUFF_SZ	= IMG_W * IMG_H * 3;

int main(int nargs, char *args[]) {

	if(nargs != 2) {

		printf("Give the sun path\n");

		return -1;

	}


	gtSocket::Client client;
	if(!client.init(args[1], 1)) {

		printf("%s\n", client.getError().c_str());

		return -1;

	}

	printf("Establish connection...");
	fflush(stdout);

	if(!client.start()) {

		printf("%s\n", client.getError().c_str());

		return -1;

	}

	printf("ok\n");

	cv::Mat imgBGR(IMG_H, IMG_W, CV_8UC3);

	const int sleepMs = 4;

	int bgr[3] = {0, 0, 0};
	int *ptr = &bgr[0];
	int inc = 1;

	int x = 0;
	int y = 0;
	int speed_x = 1;
	int speed_y = 1;

	while(1) {

		client.send((const char *)imgBGR.data, BUFF_SZ);

		cv::rectangle(imgBGR,
					  cv::Point(0, 0),
					  cv::Point(IMG_W, IMG_H),
					  cv::Scalar(bgr[0], bgr[1], bgr[2]),
					  CV_FILLED);

		cv::circle(imgBGR,
				   cv::Point(x, y),
				   10,
				   cv::Scalar(bgr[2], bgr[1], bgr[0]),
				   3,
				   CV_AA);

		if(*ptr + inc > 255 || *ptr < 0) {

			inc *= -1;

			if(ptr == &bgr[2]) {

				ptr = &bgr[0];

			}
			else {

				ptr += 1;

			}
			
		}

		*ptr = *ptr + inc;

		x += speed_x;
		y += speed_y;
		if(x > IMG_W || x < 0) {

			x -= speed_x;
			speed_x *= -1;

		}
		if(y > IMG_H || y < 0) {

			y -= speed_y;
			speed_y *= -1;

		}

		usleep(1000 * sleepMs);

	}

	return 0;

}

