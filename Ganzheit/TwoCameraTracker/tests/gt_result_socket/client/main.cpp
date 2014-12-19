#include "Client.h"
#include <stdio.h>
#include <opencv2/imgproc/imgproc.hpp>
#include "BinaryResultParser.h"
#include <vector>


void fillBuffer(char *buffer, const cv::Mat &imgBGR, const ResultData &resData);


static const int IMG_W = 640;
static const int IMG_H = 480;

static const int32_t N_GLINTS = 3;
static const int32_t imgSz = 4 + IMG_W * IMG_H * 3;
static const int32_t resDataSz = BinaryResultParser::MIN_BYTES + N_GLINTS*2*4;
static const int32_t bufferSz = imgSz + resDataSz;



int main(int nargs, char *args[]) {

	if(nargs != 2) {

		printf("Give the sun path\n");

		return -1;

	}


	gtSocket::Client client;
	if(!client.init(args[1])) {

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

	// image to send
	cv::Mat imgBGR(IMG_H, IMG_W, CV_8UC3);

	// results to send
	ResultData resData;
	resData.bTrackSuccessfull = true;
	resData.id = 1L;
	resData.timestamp = 189123;
	resData.trackDurMicros = 4;
	resData.ellipsePupil = cv::RotatedRect(cv::Point2f(300, 300), cv::Size(90, 67), 67);
	resData.corneaCentre = cv::Point3d(0, 0, 0);
	resData.pupilCentre = cv::Point3d(0, 0, 0);
	resData.scenePoint = cv::Point2d(0, 0);
	resData.listGlints.resize(3);
	resData.listGlints[0] = cv::Point2d(33, 33);
	resData.listGlints[1] = cv::Point2d(44, 44);
	resData.listGlints[2] = cv::Point2d(55, 55);


	/*********************************************************
	 * Buffer to send, consists of the image and the results
	 *********************************************************/

	// the buffer to send
	char *bufferSend = new char[bufferSz];

	// how long to sleep, in the main loop
	const int sleepMs = 4;

	// image color
	int bgr[3] = {0, 0, 0};
	int *ptr = &bgr[0];
	int inc = 1;

	// we move the scenePoint, these are the speeds
	int speed_x = 1;
	int speed_y = 1;

	// main loop
	while(1) {

		// fill the send buffer with data
		fillBuffer(bufferSend, imgBGR, resData);

		// send it
		client.send(bufferSend, bufferSz);

		// draw an altering color on the image
		cv::rectangle(imgBGR,
					  cv::Point(0, 0),
					  cv::Point(IMG_W, IMG_H),
					  cv::Scalar(bgr[0], bgr[1], bgr[2]),
					  CV_FILLED);

		// change color of the image
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

		// move the scene point
		resData.scenePoint.x += speed_x;
		resData.scenePoint.y += speed_y;
		if(resData.scenePoint.x > IMG_W || resData.scenePoint.x < 0) {

			resData.scenePoint.x -= speed_x;
			speed_x *= -1;

		}
		if(resData.scenePoint.y > IMG_H || resData.scenePoint.y < 0) {

			resData.scenePoint.y -= speed_y;
			speed_y *= -1;

		}

		// sleep
		usleep(1000 * sleepMs);

	}

	delete[] bufferSend;

	return 0;

}


void fillBuffer(char *buffer, const cv::Mat &imgBGR, const ResultData &resData) {

	/***************************************************
	 * Image
	 ***************************************************/
	uint32_t nImageBytes = IMG_W * IMG_H * 3;
	buffer[0] = nImageBytes  & 0x000000FF;
	buffer[1] = (nImageBytes & 0x0000FF00) >> 8;
	buffer[2] = (nImageBytes & 0x00FF0000) >> 16;
	buffer[3] = (nImageBytes & 0xFF000000) >> 24;

	memcpy(buffer + 4, imgBGR.data, nImageBytes);


	/***************************************************
	 * Track results
	 ***************************************************/
	buffer += 4 + nImageBytes;

	std::vector<char> resDataBuffer;
	BinaryResultParser::resDataToBuffer(resData, resDataBuffer);
	memcpy(buffer, resDataBuffer.data(), resDataSz);

}

