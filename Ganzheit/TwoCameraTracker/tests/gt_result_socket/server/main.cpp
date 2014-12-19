#include "Server.h"
#include <stdio.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "BinaryResultParser.h"


static const int IMG_W = 640;
static const int IMG_H = 480;


int main(int nargs, char *args[]) {

	if(nargs != 2) {

		printf("Give the sun path\n");

		return -1;

	}


	// 4-byte size info + image data
	const int imgDataSz = 4 + IMG_W * IMG_H * 3;
	char *buffer = new char[imgDataSz];


	gtSocket::Server server;
	if(!server.init(args[1])) {

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

	ResultData resData;

	std::string strWin("server");
	while(1) {

		/***********************************************************
		 * Image data
		 ***********************************************************/
		int nRead = server.receive(buffer, imgDataSz, true);
		if(nRead != imgDataSz) {

			printf("Got %d requested %d\n", nRead, imgDataSz);

			break;

		}

		// see how many image bytes there are
		int nImageBytes = (0x000000FF & buffer[0])       |
						  (0x000000FF & buffer[1]) << 8  |
						  (0x000000FF & buffer[2]) << 16 |
						  (0x000000FF & buffer[3]) << 24;

		// compare to the desired amount
		if(nImageBytes != IMG_W * IMG_H * 3) {

			printf("Read %d image bytes, required %d\n", nImageBytes, IMG_W * IMG_H * 3);

			break;

		}

		// image data
		const char *imgData = buffer + 4;

		// image header
		cv::Mat imgBGR(
			IMG_H,
			IMG_W,
			CV_8UC3,
			(void *)imgData
		);


		/***********************************************************
		 * Track results
		 ***********************************************************/

		// read the size of the results
		nRead = server.receive(buffer, 4, true);

		int nDataBytes = (0x000000FF & buffer[0])       |
						 (0x000000FF & buffer[1]) << 8  |
						 (0x000000FF & buffer[2]) << 16 |
						 (0x000000FF & buffer[3]) << 24;

		if(nDataBytes < BinaryResultParser::MIN_BYTES) {

			printf("Read %d data bytes. Too less because the min is: %d\n",
				   nDataBytes,
				   BinaryResultParser::MIN_BYTES);

			break;

		}

		// compute the number of bytes to read.
		// -4, because, the nDataBytes is included in the data size
		int nToRead = nDataBytes - 4;

		// read the data
		nRead = server.receive(buffer + 4, nToRead, true);

		if(nRead != nToRead) {
			printf("Error reading results: read %d, requested %d\n", read, nToRead);
			break;
		}

		bool success = BinaryResultParser::parsePacket(buffer, nDataBytes, resData);
		if(!success) {

			printf("BinaryResultParser::parsePacket() failed\n");

			break;

		}

		cv::circle(imgBGR,
				   resData.scenePoint,
				   10,
				   cv::Scalar(255, 255, 255),
				   3,
				   CV_AA);

		cv::imshow(strWin, imgBGR);

		cv::waitKey(1);

	}

	delete[] buffer;


	return 0;

}

