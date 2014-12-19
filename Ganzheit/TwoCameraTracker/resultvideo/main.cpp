#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "BinaryResultParser.h"
#include "Camera.h"
#include "CalibDataReader.h"
#include "ResultStreamer.h"
#include "InputParser.h"
#include <sys/time.h>
#include <algorithm>



/******************************************************************************
 * Prototypes
 ******************************************************************************/

static void handleKeyboard(int key);
static bool handleInputParameters(int  argc, const char **args);
static bool handleParameter(const ParamAndValue &pair);
static void printUsageInfo();
static void draw(cv::Mat &imgEye, cv::Mat &imgScene, const ResultData &data);


/******************************************************************************
 * Globals
 ******************************************************************************/

// main loop state
static bool b_running = true;

// pause state
static bool b_paused = false;

/*
 * Display the images on separate windows. Only in
 * effect if output frames are being generated
 */
static bool bShowFrames = false;

static bool bPrintHelp = false;

/*
 * Gaze tracking might have been performed for flipped images. However,
 * the stored eye video stream is never flipped, and therefore when displaying
 * the results, the eye image must be flipped.
 */
static bool bFlipY = false;

ResultStreamer resStreamer;

std::string inputFolder;
std::string outputFolder;
bool bBurnVideos = false;




#include <algorithm>
class GazeArray {

public:

    enum Limit {
        N_POINTS = 15
    };

    GazeArray() {
        indCurr = 0;
    }

    void add(const cv::Point2d &p) {

        if(vec.size() < N_POINTS) {
            vec.push_back(p);

        }
        else {
            vec[indCurr] = p;
            indCurr = (indCurr + 1) % N_POINTS;
        }

    }

    const std::vector<cv::Point2d> &getVector() {
        return vec;
    }

    void removeOldest() {

        if(vec.size() == 0) {return;}

        std::vector<cv::Point2d>::iterator it = vec.begin();
        it += indCurr;
        vec.erase(it);

        int sugg = indCurr - 1;
        if(sugg >= 0) {
            indCurr = sugg;
        }
        else {
            indCurr = std::max((int)0, (int)(vec.size() - 1));
        }

    }

private:

    std::vector<cv::Point2d> vec;
    int indCurr;

};

GazeArray scenePointArray;



int main(const int argc, const char **args) {

	if(argc < 2) {

        printUsageInfo();

		return -1;

	}

    if(!handleInputParameters(argc, args)) {

        printUsageInfo();

        return -1;

    }

    if(bPrintHelp) {

        printUsageInfo();

        return 0;

    }


    if(inputFolder.empty()) {

        printf("-i <input_folder> Must be defined\n");

        printUsageInfo();

        return -1;
    }


    if(!resStreamer.init(inputFolder)) {

        return -1;

    }



    cv::VideoWriter videoWriterEye;
    cv::VideoWriter videoWriterScene;

    int fps = resStreamer.getFps();

    if(!outputFolder.empty()) {

        //        int ex = resStreamer.getFourCC();
        int ex = CV_FOURCC('P','I','M','1');

        cv::Size imgDim = resStreamer.getDim();

        videoWriterEye.open(outputFolder + "camera1.mjpg" , ex, fps, imgDim, true);
        if(!videoWriterEye.isOpened()) {
            return false;
        }

        videoWriterScene.open(outputFolder + "camera2.mjpg" , ex, fps, imgDim, true);
        if(!videoWriterScene.isOpened()) {
            return false;
        }

        bBurnVideos = true;

    }
    else {

        bShowFrames = true;

    }


	/**********************************************************************
	 * Initialize the main window
	 *********************************************************************/

	const std::string strWinEye("Eye window");
	const std::string strWinScene("Scene window");

    if(bShowFrames) {
        cv::namedWindow(strWinEye, CV_WINDOW_AUTOSIZE);
        cv::namedWindow(strWinScene, CV_WINDOW_AUTOSIZE);
    }

	// displayed image
	cv::Mat imgEye, imgScene;


    struct timeval t1;
    gettimeofday(&t1, NULL);


	/**********************************************************************
	 * Main loop
	 *********************************************************************/
	while(b_running) {

        ResultData data;

		if(!b_paused) {

            int ret = resStreamer.get(imgEye, imgScene, data);

            switch(ret) {

                case ResultStreamer::STREAM_FINISHED: {

                    printf("stream ended\n");

                    return 0;

                }

                case ResultStreamer::STREAM_ERROR: {

                    printf("Stream error\n");
                    return -1;

                }

                default: break;

            }

		}


        if(bFlipY) {
            cv::flip(imgEye, imgEye, 1);
        }

		// analyse and draw the results
		// TODO: add the scene image
		draw(imgEye, imgScene, data);

        if(bShowFrames) {

            // get the key state
            int periodMs = 1000.0  / (double)fps;

            struct timeval t2;
            gettimeofday(&t2, NULL);
            long dSec = t2.tv_sec - t1.tv_sec;
            long dUsec = t2.tv_usec - t1.tv_usec;

            int dMillis = (int)(1000.0 * dSec + dUsec / 1000.0 + 0.5);
            int dur = std::max(2, periodMs - dMillis);

            t1 = t2;

            int key = cv::waitKey(dur);

            handleKeyboard(key);


            // show the frames
            cv::imshow(strWinEye, imgEye);
            cv::imshow(strWinScene, imgScene);

        }


        if(bBurnVideos) {
            videoWriterEye   << imgEye;
            videoWriterScene << imgScene;
        }

	}

	return EXIT_SUCCESS;

}


// TODO: add the scene image
void draw(cv::Mat &imgEye, cv::Mat &imgScene, const ResultData &data) {

	/************************************************************
	 * All contours
	 ************************************************************/

	// get the contours
	const std::vector<std::vector<cv::Point> > &listContours = data.listContours;

	if(listContours.size() > 0) {

		cv::drawContours(imgEye,					// opencv image
						 listContours,				// list of contours to be drawn
                         -1,							// draw all contours in the list
						 cv::Scalar(255, 255, 0),	// colour
						 2,							// thickness
						 CV_AA);					// line type

	}


    if(!data.bTrackSuccessfull) {
        //        return;
    }


	/*************************************************************************
	 * Draw the pupil ellipse and its center
	 *************************************************************************/
	const cv::RotatedRect &pe = data.ellipsePupil;
	const cv::Point2f cpf = pe.center;
	cv::ellipse(imgEye, pe, cv::Scalar(0, 0, 255), 3, CV_AA);

	const cv::Point pupil_centre(cpf.x, cpf.y);
	cv::circle(imgEye, pupil_centre, 3,
			   cv::Scalar(0, 0, 255), CV_FILLED, CV_AA);


	/*************************************************************************
	 * Draw the corneal reflections
	 *************************************************************************/
	const std::vector<cv::Point2d> &crs = data.listGlints;

	for(size_t i = 0; i < crs.size(); ++i) {
		cv::circle(imgEye, cv::Point(crs[i].x, crs[i].y),
				   3, cv::Scalar(0, 200, 0), CV_FILLED, CV_AA);
	}


	/************************************************************
	 * Gaze vector
	 ************************************************************/
	cv::line(imgEye,
			 data.gazeVecStartPoint2D,
			 data.gazeVecEndPoint2D,
			 cv::Scalar(0, 0, 255),
			 1);


    /*************************************************************************
     * Draw the mapped scene point
     *************************************************************************/
    const cv::Point2d &scenePoint = data.scenePoint;

    if(data.bTrackSuccessfull) {
        scenePointArray.add(scenePoint);
    }
    else {
        scenePointArray.removeOldest();
    }
    const std::vector<cv::Point2d> &vec = scenePointArray.getVector();

    for(int i = 0; i < (int)vec.size(); ++i) {

        cv::circle(imgScene,
                   vec[i],
                   10,
                   cv::Scalar(255,0,0),
                   2,
                   CV_AA);
	
    }

    if(data.bTrackSuccessfull) {
        cv::circle(imgScene, scenePoint, 10, cv::Scalar(0,0,255), 2, CV_AA);
    }

}


void handleKeyboard(int key) {

	if((char)key == 27) {
		b_running = false;
	}

	if((char)key == 'p') {
		b_paused = !b_paused;
	}

}


bool handleInputParameters(int  argc, const char **args) {

    std::vector<ParamAndValue> argVec;
    if(!parseInput(argc, args, argVec)) {
        printf("handleInputParameters(): Error parsing input\n");
        return false;
    }


    for(int i = 0; i < (int)argVec.size(); ++i) {

        if(!handleParameter(argVec[i])) {

            printf("-%s %s not defined\n", argVec[i].name.c_str(), argVec[i].value.c_str());
            return false;

        }

    }


    return true;

}


bool handleParameter(const ParamAndValue &pair) {

    if(pair.name == "i") {

        inputFolder = pair.value;

    }

    else if(pair.name == "o") {

        outputFolder = pair.value;

    }

    else if(pair.name == "h" || pair.name == "help") {

        bPrintHelp = true;

    }

    else if(pair.name == "g") {

        bShowFrames = true;

    }

    else if(pair.name == "f") {

        bFlipY = true;

    }

    else {
        return false;
    }


    return true;

}


void printUsageInfo() {

    printf("Usage:\n"
           "  ./result [option arguments]\n"
           "  option arguments:\n"
           "      [-i <input_folder>]  Folder containing partX sub-folders. Must always be defined\n"
           "      [-o <output_folder>] Where to create the burned video files\n"
           "      [-g]                 Display videos. Works only if -o option is in use\n"
           "      [-h]                 Display help\n"
           "      [-help]              Same as -h\n"
           "      [-f]                 Flip the eye image along the y-axis\n"
           );

}

