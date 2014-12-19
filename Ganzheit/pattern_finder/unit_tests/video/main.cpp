#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>
#include <stdlib.h>

#include "PupilTracker.h"
#include "group.h"
#include "settingsIO.h"
#include "localTrackerSettings.h"
#include "trackerSettings.h"

class TextAnimator {

public:

    enum {
        MAX_COUNT = 100
    };

    TextAnimator() {

        count = 0;
        fontScale = 3.0;

    }

    void reset() {
        count = 0;
        fontScale = 3.0;
    }

    void setText(const std::string &_txt) {

        txt = _txt;

    }

    void update(cv::Mat &imgBgr) {

        if(count < MAX_COUNT) {

            ++count;
        
            cv::putText(imgBgr,
                        txt,
                        cv::Point(55, 55),
                        cv::FONT_HERSHEY_SIMPLEX,
                        fontScale,
                        cv::Scalar(0, 0, 255),
                        3,
                        CV_AA);

            fontScale *= 0.98;

        }

    }

private:

    int count;
    int maxCount;

    double fontScale;
    std::string txt;

};



/******************************************************************************
 * Prototypes
 ******************************************************************************/

static void handleKeyboard();
static void analyse_and_draw(cv::Mat &img_bgr, cv::Mat &img_gray);
static bool openStream(cv::VideoCapture &vc, const char *);


/******************************************************************************
 * Globals
 ******************************************************************************/

// pupil tracker
gt::PupilTracker *tracker = NULL;

// main loop state
bool b_running = true;

// pause state
bool b_paused = false;

TextAnimator textAnimator;


int main(const int argc, const char **argv) {

	if(argc != 2) {
		printf("Usage:\n  video <file_name>\n");
		return -1;
	}

    cv::VideoCapture videoStreamer;

	// create the video streamer
    if(!openStream(videoStreamer, argv[1])) {
		printf("Could not stream %s\n", argv[1]);
        return -1;
    }


    textAnimator.setText("begin");


	/**********************************************************************
	 * Initialize the main window
	 *********************************************************************/

	const std::string win_name("Main");
	cv::namedWindow(win_name, CV_WINDOW_AUTOSIZE);

	// displayed image
	cv::Mat _img_bgr;
    cv::Mat img_bgr;


	// analysed image
	cv::Mat img_gray;

	// Read settings from file
	{
		SettingsIO settingsFile("default.xml");
		LocalTrackerSettings localSettings;
		localSettings.open(settingsFile);
		trackerSettings.set(localSettings);
	}


	// create the pupil tracker
	tracker = new gt::PupilTracker();
	tracker->set_nof_crs(6);


	/**********************************************************************
	 * Main loop
	 *********************************************************************/
	while(b_running) {

		handleKeyboard();

		if(!b_paused) {
			// grab a frame
			videoStreamer >> _img_bgr;

		}

		if(_img_bgr.empty()) {

            if(!openStream(videoStreamer, argv[1])) {
                printf("Stream ended, but could not be reopened\n");
                break;
            }

            textAnimator.setText("reset");
            textAnimator.reset();

			continue;

		}


		_img_bgr.copyTo(img_bgr);

		// convert to gray
		cv::cvtColor(img_bgr, img_gray, CV_BGR2GRAY);

		// analyse and draw the results
		analyse_and_draw(img_bgr, img_gray);

        textAnimator.update(img_bgr);

		// show the image
		cv::imshow(win_name, img_bgr);

	}

	delete tracker;

	return EXIT_SUCCESS;

}


void analyse_and_draw(cv::Mat &img_bgr, cv::Mat &img_gray) {

	/*************************************************************************
	 * Track the pupil and the glints
	 *************************************************************************/
	if(!tracker->track(img_gray)) {
		return;
	}


	/*************************************************************************
	 * Draw the pupil ellipse and its center
	 *************************************************************************/
	const cv::RotatedRect *pe = tracker->getEllipsePupil();
	const cv::Point2f cpf = pe->center;
	cv::ellipse(img_bgr, *pe, cv::Scalar(0, 0, 255), 3, CV_AA);

	const cv::Point pupil_centre(cpf.x, cpf.y);
	cv::circle(img_bgr, pupil_centre, 3,
			   cv::Scalar(0, 0, 255), CV_FILLED, CV_AA);


	/*************************************************************************
	 * Draw the corneal reflections
	 *************************************************************************/
	const std::vector<cv::Point2d> &crs = tracker->getCornealReflections();

	for(size_t i = 0; i < crs.size(); ++i) {
		cv::circle(img_bgr, cv::Point(crs[i].x, crs[i].y),
				   3, cv::Scalar(0, 100, 0), CV_FILLED, CV_AA);
	}


	/*************************************************************************
	 * Try to identify the glints
	 *************************************************************************/

	// copy to an interger point vector
	std::vector<cv::Point> points(crs.size());
	for(size_t i = 0; i < crs.size(); ++i) {
		points[i] = cv::Point(crs[i].x, crs[i].y);
	}

	group::GroupManager grp;

	if(grp.identify(points, pupil_centre)) {

		const group::Configuration &configuration = grp.getBestConfiguration();
		const std::vector<group::Element> &elements = configuration.elements;


		/*********************************************************************
		 * Draw the glint labels
		 *********************************************************************/
		for(size_t i = 0; i < elements.size(); ++i) {

			char txt[10];
			sprintf(txt, "%d", elements[i].label);

			cv::putText(img_bgr, std::string(txt), points[i],
						cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 255, 0));

		}

	}

}


void handleKeyboard() {

	// get the key state
	const int key = cv::waitKey(5);

	if((char)key == 27) {
		b_running = false;
	}

	if((char)key == 'p') {
		b_paused = !b_paused;
	}

}


bool openStream(cv::VideoCapture &videoStreamer, const char *file) {

    videoStreamer.release();

    static const char *target = "cam=";
    static const size_t targetLen = strlen(target);

    const char *ptr = strstr(file, target);
    if(ptr != NULL && ptr == file && strlen(file) > targetLen) {

        int deviceID = atoi(ptr + targetLen);

        videoStreamer.open(deviceID);

    }
    else {
        videoStreamer.open(file);
    }

    return videoStreamer.isOpened();

}

