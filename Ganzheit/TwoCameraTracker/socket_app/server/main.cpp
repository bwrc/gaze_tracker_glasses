#include "Server.h"
#include <stdio.h>
#include <pthread.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "DataSink.h"
#include "jpeg.h"
#include "BinaryResultParser.h"
#include "DataQueue.h"


/**********************************************************************
 * This class is the server receiving data from the gaze tracker
 * client. The data contains eye and the scene camera frames,
 * as well as the gaze tracking results. The data can be assumed
 * to be received in the following way:
 *
 * 1. Frame 1 always arrives before frame 2.
 * 2. Frame 2 always arrives immediately after frame 1
 * 3. Data after frame 2 can be either the tracking results or frame 1
 * 4. The results do _not_ necessarily correspond to the last
 *    frames, but can be related to older frames.
 **********************************************************************/


void drawResults(cv::Mat &imgEye, cv::Mat &imgScene, const ResultData &resData);
bool receiveFromClient(gtSocket::Server &server, char *buffer);
bool readFrames(gtSocket::Server &server,
				char *buffer,
				CameraFrameExtended **f1,
				CameraFrameExtended **f2);


static const int IMG_W = 640;
static const int IMG_H = 480;


/*
 * If the server nothing is being displayed on the windows after the
 * connection has been established, the reason might be that cv::waitKey()
 * is not waiting long enough to be able to handle events. Increase this
 * value in such case.
 */
static const int WAIT_DURATION_MS = 1;


/*
 * The maximum size of the receivable data:
 *
 *    type + size + image
 *
 * which is assumed to be one of the frames, when the compression
 * rate is 0.
 */
static const int BUFF_SZ = 4 + 4 + IMG_W * IMG_H * 3;


/* Used to store frames and track results, for drawing purposes only */
DataQueue GUIQueue;



pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
volatile bool bThreadRunning = false;


void *threadFnct(void *ptr) {

	const char *serverFile = (char *)ptr;

	gtSocket::Server server;

	// initialise the server
	if(!server.init(serverFile)) {

		printf("%s\n", server.getError().c_str());

		pthread_exit(NULL);

	}

	printf("Establish connection...");
	fflush(stdout);

	// start the server
	if(!server.start()) {

		printf("%s\n", server.getError().c_str());

		pthread_exit(NULL);

	}

	printf("ok\n");


	char *buffer = new char[BUFF_SZ];
	bThreadRunning = true;

	while(bThreadRunning) {

		if(!receiveFromClient(server, buffer)) {
			bThreadRunning = false;
		}

	}

	delete[] buffer;


	pthread_exit(NULL);

}


bool createThread(pthread_t &thread, const char *serverFile) {

	// thread attributes
	pthread_attr_t attr;

	/*
	 * initialize and set thread detached attribute.
	 * On some systems the thread will not be created
	 * as joinable, so do it explicitly. It is more
	 * portable this way
	 */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	// create the thread
	int success = pthread_create(&thread, &attr, &threadFnct, (void *)serverFile);

	// destroy the attributes
	pthread_attr_destroy(&attr);

	// was the thread creation successfull...
	if(success != 0) {

	    // ...nope...

	    return false;
	}

	// ...yes it was ...
	return true;

}


int main(int nargs, char *args[]) {

	// check the number of arguments
	if(nargs != 2) {

		printf("Give the sun path\n");

		return -1;

	}

	/***************************************
	 * Create the receiver thread
	 ***************************************/
	pthread_t thread;
	if(!createThread(thread, args[1])) {
		return -1;
	}


	// window names
	std::string strWinEye("eye camera");
	std::string strWinScene("scene camera");

	// main loop
	while(1) {

		pthread_mutex_lock(&mutex);

			// get the oldest element, must be released after use
			QueueElement el = GUIQueue.getElement();

		pthread_mutex_unlock(&mutex);

		// check that the element is not empty
		if(!el.empty()) {

			// make OpenCV headers
			cv::Mat imgEye(
				IMG_H,
				IMG_W,
				CV_8UC3,
				el.f1->data
			);

			cv::Mat imgScene(
				IMG_H,
				IMG_W,
				CV_8UC3,
				el.f2->data
			);


			// result might be missing altough el.empty() returns false
			if(el.res != NULL) {

				drawResults(imgEye, imgScene, *el.res);

			}

			// draw the results
			cv::imshow(strWinEye, imgEye);
			cv::imshow(strWinScene, imgScene);

			// deallocate memory
			el.release();

		}


		// let OpenCV poll events
		char key = cv::waitKey(WAIT_DURATION_MS);

		if(key == 27) {
			break;
		}

	}

	bThreadRunning = false;
	pthread_join(thread, NULL);


	/******************************************
	 * exit the main thread
	 ******************************************/
	pthread_exit(NULL);


	return 0;

}


bool receiveFromClient(gtSocket::Server &server, char *buffer) {

	/****************************************************
	 * Read the type
	 ****************************************************/
	int nRead = server.receive(buffer, 4, true);

	int32_t type = (0x000000FF & buffer[0])	       |
				  ((0x000000FF & buffer[1]) << 8)  |
				  ((0x000000FF & buffer[2]) << 16) |
				  ((0x000000FF & buffer[3]) << 24);

	if(type != DataContainer::TYPE_TRACK_RESULTS &&
	   type != DataContainer::TYPE_FRAME1) {

		printf("type %d not valid\n", type);
		return false;

	}


	/**************************************************
	 * Frame 1 ready, followed by frame 2
	 **************************************************/
	if(type == DataContainer::TYPE_FRAME1) {

		CameraFrameExtended *f1, *f2;

		bool success = readFrames(server, buffer, &f1, &f2);

		if(!success) {
			return false;
		}

		// TODO: save frames

		pthread_mutex_lock(&mutex);
			GUIQueue.addFrames(f1, f2);
		pthread_mutex_unlock(&mutex);

	}

	/**************************************************
	 * Results ready
	 **************************************************/
	else {

		server.receive(buffer, 4, true);

		int32_t size = (0x000000FF & buffer[0])		   |
					  ((0x000000FF & buffer[1]) << 8)  |
					  ((0x000000FF & buffer[2]) << 16) |
					  ((0x000000FF & buffer[3]) << 24);

		if(size < 4) {

			printf("size %d < 4\n", size);
			return false;

		}

		if(size > BUFF_SZ) {

			printf("Too much inconming data\n");
			return false;

		}

		// -4 because size has already been read
		int nToRead = size - 4;
		nRead = server.receive(buffer + 4, nToRead, true);
		if(nRead != nToRead) {

			printf("Got %d requested %d\n", nRead, nToRead);
			return false;

		}


		ResultData *res = new ResultData;
		if(!BinaryResultParser::parsePacket(buffer, size, *res)) {
			printf("Could not parse result packet\n");
			delete res;
			return false;
		}

		// TODO: save results

		pthread_mutex_lock(&mutex);
			GUIQueue.addResults(res);
		pthread_mutex_unlock(&mutex);

	}

	return true;

}


bool readFrames(gtSocket::Server &server,
				char *buffer,
				CameraFrameExtended **f1,
				CameraFrameExtended **f2) {

	*f1 = *f2 = NULL;

	CameraFrameExtended **imgs[2] = {f1, f2};

	for(int i = 0; i < 2; ++i) {

		server.receive(buffer, 4, true);

		int32_t size = (0x000000FF & buffer[0])		   |
					  ((0x000000FF & buffer[1]) << 8)  |
					  ((0x000000FF & buffer[2]) << 16) |
					  ((0x000000FF & buffer[3]) << 24);

		if(size < 4) {

			printf("size %d < 4\n", size);
			delete *f1;
			delete *f2;
			return false;

		}

		if(size > BUFF_SZ) {

			printf("Too much inconming data\n");
			delete *f1;
			delete *f2;
			return false;

		}


		/*
		 * The input contains:
		 *   type                                      | read previously
		 *   size, including this 4-byte size info     | read previously
		 *   format                                    | will read next
		 *   id                                        | will read next
		 *   data                                      | will read next
		 */

		// -4 because size has already been read
		int nToRead = size - 4;
		int nRead = server.receive(buffer, nToRead, true);
		if(nRead != nToRead) {

			printf("Got %d requested %d\n", nRead, nToRead);
			delete *f1;
			delete *f2;
			return false;

		}

		int32_t format =  (0x000000FF & buffer[0])		  |
						 ((0x000000FF & buffer[1]) << 8)  |
						 ((0x000000FF & buffer[2]) << 16) |
						 ((0x000000FF & buffer[3]) << 24);

		int32_t id =      (0x000000FF & buffer[4])		  |
						 ((0x000000FF & buffer[5]) << 8)  |
						 ((0x000000FF & buffer[6]) << 16) |
						 ((0x000000FF & buffer[7]) << 24);


		CameraFrameExtended **frame = imgs[i];

		unsigned char *destData = new unsigned char[3*IMG_W*IMG_H];

		JPEG_Decompressor jpegDec;
		bool success = jpegDec.decompress((const unsigned char *)buffer + 8, size - 12, destData);

		if(!success) {

			printf("Error decompressing JPEG frame\n");
			delete *f1;
			delete *f2;
			return false;

		}


		*frame = new CameraFrameExtended(id,             // id
										 NULL,           // res
										 IMG_W,          // w,
										 IMG_H,          // h,
										 3,              // bytes per pixel,
										 destData,       // data,
										 3*IMG_W*IMG_H,  // size,
										 (Format)format, // format,
										 false,          // copy data,
										 true);          // become parent


		if(i == 0) {

			server.receive(buffer, 4, true);

			// read type
			int32_t type = (0x000000FF & buffer[0])		   |
						  ((0x000000FF & buffer[1]) << 8)  |
						  ((0x000000FF & buffer[2]) << 16) |
						  ((0x000000FF & buffer[3]) << 24);

			if(type != DataContainer::TYPE_FRAME2) {
				delete *f1;
				delete *f2;
				return false;
			}

		}

	}

	return true;

}


void drawResults(cv::Mat &imgEye, cv::Mat &imgScene, const ResultData &resData) {

	/************************************************************
	 * All contours
	 ************************************************************/
	const std::vector<std::vector<cv::Point> > &listContours = resData.listContours;

	if(listContours.size() > 0) {

		cv::drawContours(imgEye,					// opencv image
						 listContours,				// list of contours to be drawn
						-1,							// draw all contours in the list
						 cv::Scalar(255, 255, 0),	// colour
						 2,							// thickness
						 CV_AA);					// line type, antialiased

	}


	/************************************************************
	 * Pupil ellipse
	 ************************************************************/
	const cv::RotatedRect &ellipsePupil = resData.ellipsePupil;
	cv::ellipse(imgEye,					// opencv image
				ellipsePupil,			// pupil ellipse
				cv::Scalar(255, 0, 0),	// colour
				2,						// thickness
				CV_AA);					// line type anti-aliased


	/************************************************************
	 * Corneal reflections
	 ************************************************************/
	const std::vector<cv::Point2d> &crs = resData.listGlints;
	if(crs.size()) {
		if(crs[0].x != -1) {
			for(int i = 0; i < (int)crs.size(); ++i) {
				int x = (int)(crs[i].x + 0.5);
				int y = (int)(crs[i].y + 0.5);
				int x1 = x - 5;
				int x2 = x + 5;
				int y1 = y - 5;
				int y2 = y + 5;
				cv::line(imgEye, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0), 2);
				cv::line(imgEye, cv::Point(x2, y1), cv::Point(x1, y2), cv::Scalar(0, 255, 0), 2);
			}
		}
	}


	/************************************************************
	 * Gaze vector
	 ************************************************************/
	cv::line(imgEye,
			 resData.gazeVecStartPoint2D,
			 resData.gazeVecEndPoint2D,
			 cv::Scalar(0, 0, 255),
			 1);


	/***************************************************************
	 * Draw the point in the scene frame
	 **************************************************************/
	cv::circle(imgScene, resData.scenePoint, 10, cv::Scalar(0, 0, 255), 2, CV_AA);

}

