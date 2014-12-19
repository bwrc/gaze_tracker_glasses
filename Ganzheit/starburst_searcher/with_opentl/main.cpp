#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <cstdlib>
#include <cstdio>

#include "Timer.h"
#include "PupilTracker.h"
#include "SceneTracker.h"

#include "settingsIO.h"
#include "trackerSettings.h"
#include "localTrackerSettings.h"
#include "settingsPanel.h"
#include "libopentld/tld/TLD.h"

#include "group.h"



static const double PI = 3.14;

static const int downsampleFactor = 2;


using namespace std;

static const double MAX_FONT_SCALE = 3.0;

class TextAnimator {

public:

    enum {
        DUR_ANIMATION_MS = 4000
    };

    TextAnimator() {

        reset();

    }

    void reset() {

        gettimeofday(&tThen, NULL);
        fontScale = MAX_FONT_SCALE;

    }

    void setText(const std::string &_txt) {

        txt = _txt;

    }

    void update(cv::Mat &imgBgr) {

        struct timeval tNow;
        gettimeofday(&tNow, NULL);
        long dSec = tNow.tv_sec - tThen.tv_sec;
        long dUsec = tNow.tv_usec - tThen.tv_usec;

        double millis = 1000.0 * dSec + dUsec / 1000.0;

        if(millis <= DUR_ANIMATION_MS) {

            cv::Point pointCentre(imgBgr.cols / 2,
                                  imgBgr.rows / 2);
        
            cv::putText(imgBgr,
                        txt,
                        pointCentre,
                        cv::FONT_HERSHEY_SIMPLEX,
                        fontScale,
                        cv::Scalar(0, 0, 255),
                        3,
                        CV_AA);

            fontScale = MAX_FONT_SCALE * (DUR_ANIMATION_MS - millis) / DUR_ANIMATION_MS;

        }

    }

private:

    struct timeval tThen;

    int maxCount;

    double fontScale;
    std::string txt;

};



class ProgramState {
	public:
		ProgramState() {
			running	= true;
			paused	= false;
			change_frame = false;
			map2scene = track = false;
		}

		bool output_values;
		bool running;
		bool paused;
		bool change_frame;
		bool map2scene;
		bool track;
};


static const int ROI_W = 200;
static const int ROI_H = 200;

static const int SCENE_W = 1024;
static const int SCENE_H = 768;

gt::PupilTracker *pupilTracker = NULL;
tld::TLD *tldTracker = NULL;


void clean() {
 
    delete tldTracker;
    tldTracker = NULL;

    delete pupilTracker;
    pupilTracker = NULL;

}


void drawResulImages(bool bTrackOk,
                     const cv::Mat &img_orig_bgr,
                     const cv::Mat &img_binary,
                     cv::Mat &img_display,
                     const cv::Rect *curBB,
                     double runningTime,
                     const std::vector<group::Element> elements);

void Draw_Cross(cv::Mat &image,
                int centerx,
                int centery,
                int x_cross_length,
                int y_cross_length,
                cv::Scalar color,
                int lineWidth);

void handleKey(int key);
void mouse_callback(int event, int x, int y, int flags, void* param);
void mouse_callback_scene(int event, int x, int y, int flags, void* param);
void resize_images();


ProgramState prog_state;

SceneTracker sceneTracker;
SettingsPanel settingsPanel;

unsigned char th = 0;
cv::Rect pupil_roi;
cv::VideoCapture cap;

cv::Mat img_frame;
cv::Mat img_display;
cv::Mat img_gray;
cv::Mat img_scene;

std::vector<CalibrationPoint> calibrationPoints;


TextAnimator textAnimator;


void drawStarburst(cv::Mat & bgr_frame, const std::list<EdgePoint> & inliers, const std::list<EdgePoint> & outliers)
{

	int step = bgr_frame.step;


	if(bgr_frame.rows <= 0 || bgr_frame.cols <= 0) {
		cout << "drawStarburst() called with empty frame!\n";
		exit(1);
	}

	if (inliers.size()) {

		list<EdgePoint>::const_iterator edge;
		for (edge = inliers.begin();
			edge != inliers.end( ) ;
			edge++ ) {

			int ind = round(edge->point.y) * step + 3 * round(edge->point.x);
			if(round(edge->point.y) > bgr_frame.rows ||
				round(edge->point.x) > bgr_frame.cols) {
				cout << "Bad inlier point: " << edge->point.x << "," << edge->point.y << "\n";
				exit(1);
			}
			bgr_frame.data[ind]	= 0;
			bgr_frame.data[ind+1]	= 255;
			bgr_frame.data[ind+2]	= 0;
		}
	}

	if(outliers.size()) {

		list<EdgePoint>::const_iterator edge;
		for (edge = outliers.begin();
			edge != outliers.end() ;
			edge++) {

			int ind = round(edge->point.y) * step + 3 * round(edge->point.x);
			if(round(edge->point.y) > bgr_frame.rows ||
				round(edge->point.x) > bgr_frame.cols) {
				cout << "Bad outlier point: " << edge->point.x << "," << edge->point.y << "\n";
				exit(1);
			}
			bgr_frame.data[ind]	= 0;
			bgr_frame.data[ind+1]	= 0;
			bgr_frame.data[ind+2]	= 255;
		}
	}


	const Starburst &starburst = pupilTracker->getStarburstObject();
    cv::Point2f sp = starburst.getStartPoint();

    Draw_Cross(bgr_frame, sp.x, sp.y, 5, 5, cv::Scalar(255, 100, 100), 2);

}


void outputPointValues(cv::Mat &gray, const std::list<EdgePoint> & edge_points) {

	if (edge_points.size()) {

		int step = gray.step;

		cout << "Pixel values: ";

		std::list<EdgePoint>::const_iterator edge;
		for (edge = edge_points.begin();
			edge != edge_points.end( ) ;
			edge++ ) {

			int ptr = round(edge->point.y) * step + round(edge->point.x);
			cout << edge->point.x << ", " << edge->point.y << ", " << (int)gray.data[ptr] << endl; 
		}
	}

}


bool create_stream(cv::VideoCapture &video, const char *input) {

    video.release();

	if(strstr(input, "cam=") != NULL) {
		int device = 0;
		if(strlen(input) > 4) {
			device = atoi(input + 4);
		}
		printf("using device %d\n", device);
		video.open(device);
		video.set(CV_CAP_PROP_FRAME_WIDTH, 640);
		video.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	}
	else {
		video.open(input);
	}

	if(!video.isOpened()) {
		printf("Could not create stream\n");
		return false;
	}

	return true;
}


int main(int argc, char **argv)
{

	if(argc < 2) {

		std::cout << "Usage: " << std::string(argv[0]) <<
			" videofile [--track] [--config=filename.xml]" <<
			std::endl;

		return -1;
	}

	MyTimer timer;

	std::string configurationFilename("default.xml");
	std::string videoFilename;

	img_scene = cv::Mat::zeros(SCENE_H, SCENE_W, CV_8UC3);

	/**********************************************************************
	 * Initialize the main win
	 *********************************************************************/
	cv::namedWindow ("Main", CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback("Main", mouse_callback, NULL);

	/**********************************************************************
	 * Initialize the settings panel
	 *********************************************************************/

    std::string filePupilModel;

	// Read settings from file
	{
		SettingsIO settingsFile(configurationFilename);
		LocalTrackerSettings localSettings;
		localSettings.open(settingsFile);
		settingsPanel.setSettings(localSettings);
		settingsPanel.open(settingsFile);

		settingsFile.get("PupilTracker", "TLD_PUPIL_MODEL_FILE", filePupilModel);

		trackerSettings.set(*settingsPanel.getSettings());

	}

	settingsPanel.show();

	/**********************************************************************
	 * Handle parameters
	 *********************************************************************/

	for(int i = 1; i < argc; i++) {

		// Extract the setting name and value (or the command)
		std::string argument = argv[i];
		size_t settingLen = argument.find("=", 0);
		std::string setting, value;

		if(settingLen != string::npos) {
			setting = argument.substr(0, settingLen);
			value = argument.substr(settingLen + 1,
				argument.length());
		} else {
			setting = argument;
		}

		// Check command/setting
		if(setting == std::string("--track")) {
			prog_state.track = true;
		} else if(setting == std::string("--config")) {
			configurationFilename = value;
		} else if(setting == std::string("cam")) {
			videoFilename = argument;
		} else if(videoFilename.empty()) {
			videoFilename = setting;
		} else {
			std::cout << "Unknown argument: " << setting << std::endl;
			exit(1);
		}

	}

	if(videoFilename.empty()) {
		std::cout << "No video file specified" << std::endl;
	}


	/**********************************************************************
	 * Initialize the video stream
	 *********************************************************************/
	if (!create_stream(cap, videoFilename.c_str())){
		cout << "Could not open video stream!\n";
		return 0;
	}



	/**********************************************************************
	 * Initialize the pupil tracker
	 **********************************************************************/
	pupilTracker = new gt::PupilTracker();
	pupilTracker->set_nof_crs(6);


    /*********************************************************************
     * Initialis the TLD tracker
     *********************************************************************/
    int imgWidth  = cap.get(CV_CAP_PROP_FRAME_WIDTH);
    int imgHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT);


    cv::Rect rectCrop(trackerSettings.CROP_AREA_X,
                      trackerSettings.CROP_AREA_Y,
                      trackerSettings.CROP_AREA_W,
                      trackerSettings.CROP_AREA_H);


    if(rectCrop.width == 0    || rectCrop.height == 0 ||
       rectCrop.x >= imgWidth || rectCrop.y >= imgHeight) {

        rectCrop = cv::Rect(0, 0, imgWidth, imgHeight);

    }

    printf("%d %d %d %d\n", rectCrop.x, rectCrop.y, rectCrop.width, rectCrop.height);


    if(rectCrop.x + rectCrop.width - 1 >= imgWidth) {

        printf("Restricting the width of the cropped area\n"
               "  old: (%d, %d, %d, %d)\n",
               rectCrop.x,
               rectCrop.y,
               rectCrop.width,
               rectCrop.height);

        rectCrop.width = imgWidth - rectCrop.x;

        printf("  new: (%d, %d, %d, %d)\n",
               rectCrop.x,
               rectCrop.y,
               rectCrop.width,
               rectCrop.height);

    }

    if(rectCrop.y + rectCrop.height - 1 >= imgHeight) {

        printf("Restricting the height of the cropped area\n"
               "  old: (%d, %d, %d, %d)\n",
               rectCrop.x,
               rectCrop.y,
               rectCrop.width,
               rectCrop.height);

        rectCrop.height = imgHeight - rectCrop.y;

        printf("  new: (%d, %d, %d, %d)\n",
               rectCrop.x,
               rectCrop.y,
               rectCrop.width,
               rectCrop.height);

    }


    int wTracker = rectCrop.width;
    int hTracker = rectCrop.height;


    /*
     * It is dangerous to assume that width = widthStep. However, since tld::processImage
     * converts the image into gray scale, the step will alway be the same as the width.
     */
    tldTracker = new tld::TLD((int)(wTracker  / (double)downsampleFactor + 0.5),  // width
                              (int)(hTracker  / (double)downsampleFactor + 0.5),  // height
                              (int)(wTracker  / (double)downsampleFactor + 0.5)); // width step

    /*
     * These values were copied from the original application.
     * Tracking is more robust with these.
     */
    {

        tldTracker->trackerEnabled = true;
        tldTracker->learningEnabled = 1;

        tld::DetectorCascade *detectorCascade = tldTracker->detectorCascade;

        detectorCascade->varianceFilter->enabled     = true;
        detectorCascade->ensembleClassifier->enabled = true;
        detectorCascade->nnClassifier->enabled       = true;
        detectorCascade->useShift                    = true;
        detectorCascade->shift                       = 0.1;
        detectorCascade->minScale                    = -10;
        detectorCascade->maxScale                    = 10;
        detectorCascade->minSize                     = 25;
        detectorCascade->numTrees                    = 10;
        detectorCascade->numFeatures                 = 10;
        detectorCascade->nnClassifier->thetaTP       = 0.65;
        detectorCascade->nnClassifier->thetaFP       = 0.5;

        if(!tldTracker->readFromFile(filePupilModel.c_str())) {

            printf("TLD_PUPIL_MODEL_FILE is not defined in default.xml\n");

            clean();
            return -1;
        }

    }


	/**********************************************************************
	 * Initialize scene tracking window
	 *********************************************************************/

	if(prog_state.track) {
		cv::namedWindow ("Scene Window", CV_WINDOW_AUTOSIZE);
		cvSetMouseCallback("Scene Window", mouse_callback_scene, NULL);
		prog_state.track = true;
	}


    textAnimator.setText("begin");

	/**********************************************************************
	 * Grab images
	 *********************************************************************/

	// current and old sizes of the grabbed frame
	cv::Size cur_sz, old_sz;

	while(prog_state.running) {


		/*************************************************************
		 * Keyboard handling
		 *************************************************************/
		int key = cv::waitKey(2);

		if(key > 0) {
			handleKey(key);
		}


		trackerSettings.set(*settingsPanel.getSettings());

		if(!prog_state.paused) {

			cap >> img_frame;

			if(img_frame.empty()) {

                create_stream(cap, videoFilename.c_str());

                textAnimator.reset();
                textAnimator.setText("video reset");


				continue;

			}
		}

		else if(prog_state.change_frame) {

			prog_state.change_frame = false;

			cap >> img_frame;
			if(img_frame.empty()) {

                create_stream(cap, videoFilename.c_str());

                textAnimator.reset();
                textAnimator.setText("video reset");

				continue;

			}			

		}

		cur_sz = img_frame.size();
		if(cur_sz != old_sz) {
			resize_images();
		}
		old_sz = cur_sz;

        cv::Mat imgBgrTrack(img_frame, rectCrop);



        /*
         * Had to comment this away, because the corneal refelctions were
         * not found as well anymore.
         */
		// apply Gaussian blur to reduce noise
        //		cv::GaussianBlur(img_gray, img_gray, cv::Size(5, 5), 0);

		timer.markTime();

        if(downsampleFactor != 1) {

            cv::Mat imgBgrSmall;
            cv::Size sz((int)(imgBgrTrack.cols / (double)downsampleFactor + 0.5),
                        (int)(imgBgrTrack.rows / (double)downsampleFactor + 0.5));

            cv::resize(imgBgrTrack, imgBgrSmall, sz);

            tldTracker->processImage(imgBgrSmall);

        }
        else {

            tldTracker->processImage(imgBgrTrack);

        }


        const cv::Rect *currBB = tldTracker->getCurrBB();

		cv::cvtColor(img_frame, img_gray, CV_BGR2GRAY);

        bool bTrackOk = false;
        if(currBB != NULL) {

            cv::Point2f startPoint(currBB->x + currBB->width / 2.0f,
                                   currBB->y + currBB->height / 2.0f);

            startPoint *= downsampleFactor;

            startPoint.x += rectCrop.x;
            startPoint.y += rectCrop.y;

            bTrackOk = pupilTracker->track(img_gray, &startPoint);

        }
        else {

            bTrackOk = pupilTracker->track(img_gray);

        }


        /************************************************************
         * Assign the glints to the LEDs
         ************************************************************/
        const std::vector<cv::Point2d> &crs = pupilTracker->getCornealReflections();
        size_t sz = crs.size();
        std::vector<group::Element> elements;

        if(sz > 1) {

            group::GroupManager grp;
            const cv::RotatedRect *ellipse = pupilTracker->getEllipsePupil();
            cv::Point2f pc = ellipse->center;

            /************************************************************************
             * The GroupManager requires an integer coordinate vector. Therefore
             * the given double precision vector must be copied to such a container.
             ************************************************************************/

            std::vector<cv::Point> extracted(sz);

            for(size_t i = 0; i < sz; ++i) {
                extracted[i].x = (int)(crs[i].x + 0.5);
                extracted[i].y = (int)(crs[i].y + 0.5);
            }

            if(grp.identify(extracted, pc)) {

                const group::Configuration &config = grp.getBestConfiguration();
                elements = config.elements; // copy

            }

        }



		double runningTime = timer.getElapsed_micros();


		// .. show
		const cv::Mat &img_binary = pupilTracker->getBinaryImage();
		drawResulImages(bTrackOk, img_frame, img_binary, img_display, currBB, runningTime, elements);

        textAnimator.update(img_display);

		imshow("Main", img_display);

	}

    clean();

	return 0;

}


/*
 *     __________________________
 *     |            |            |
 *     |  img_orig  |   img_sb   |
 *     |____________|____________|
 *     |            |            |
 *     | img_binary | img_track  |
 *     |____________|____________|
 *
 */
void drawResulImages(bool bTrackOk,
					 const cv::Mat &img_orig_bgr,
					 const cv::Mat &img_binary,
					 cv::Mat &img_display,
                     const cv::Rect *currBB,
					 double runningTime,
                     const std::vector<group::Element> elements) {

    // paint it black
    cv::rectangle(img_display,
                  cv::Point(0, 0),
                  cv::Point(img_display.cols - 1, img_display.rows - 1),
                  cv::Scalar(0, 0, 0),
                  CV_FILLED);

	int w = img_orig_bgr.cols;
	int h = img_orig_bgr.rows;

	static const int border_w = 5; // please let it be odd
	int dw = 2*w + border_w;
	int dh = 2*h + border_w;

	if(img_display.cols != dw || img_display.rows != dh) {
		img_display = cv::Mat::zeros(dh, dw, CV_8UC3);
	}

	/**********************************************************************
	 * orig image as such with the TLD rect and the cropped area, if
     * defined.
	 *********************************************************************/
	{

        // copy to the displayable image
		cv::Rect roi(0, 0, w, h);
		cv::Mat d(img_display, roi);
		img_orig_bgr.copyTo(d);

        // track duration text
		char txt[128];
		sprintf(txt, "Dur %.2fms", runningTime / 1000.0);

		cv::putText(d,
					txt,
					cv::Point(10, 20),
					cv::FONT_HERSHEY_SIMPLEX,
					0.5,
					cv::Scalar(255, 0, 200),
					1,
					CV_AA);

        // TLD rectangle
        if(currBB != NULL) {

            cv::Rect r = *currBB;
            r.x       *= downsampleFactor;
            r.y       *= downsampleFactor;
            r.width   *= downsampleFactor;
            r.height  *= downsampleFactor;


            cv::Rect rectCrop = pupilTracker->getCropArea();
            r.x += rectCrop.x;
            r.y += rectCrop.y;


            double c = tldTracker->currConf;
            cv::Scalar rectangleColor(255*c, 0, (1.0-c)*255);
            cv::rectangle(d,
                          r.tl(),
                          r.br(),
                          rectangleColor,
                          8,  // line width
                          8); // line type

        }


        cv::Rect rectCrop = pupilTracker->getCropArea();

        // cropped area
        if(rectCrop.size() != img_frame.size()) {

            cv::rectangle(d,
                          rectCrop.tl(),
                          rectCrop.br(),
                          cv::Scalar(0, 0, 0),
                          8,  // line width
                          8); // line type

        }

	}


	/**************************************************************
	 * Starburst image
	 *************************************************************/
	{

        cv::Rect roi(w + border_w, 0, w, h);
		cv::Mat d(img_display, roi);

		const Starburst &starburst = pupilTracker->getStarburstObject();
		drawStarburst(d, starburst.getInlierPoints(), starburst.getOutlierPoints());

	}


	/**************************************************************
	 * Binary image
	 *************************************************************/

	if(img_binary.cols > 0 && img_binary.rows > 0) {

		cv::Rect rectRoi = pupilTracker->getROI();

		cv::Rect roi(0, h+border_w, w, h);
		cv::Mat d(img_display, roi);


		cv::Mat d_roi = cv::Mat(d, rectRoi);
		cv::Mat img_binary_roi = cv::Mat(img_binary, rectRoi);
		cv::Mat img_binary_bgr_roi;
		cv::cvtColor(img_binary_roi, img_binary_bgr_roi, CV_GRAY2BGR);
		img_binary_bgr_roi.copyTo(d_roi);

		cv::rectangle(d,
					  cv::Point(rectRoi.x, rectRoi.y),
					  cv::Point(rectRoi.x + rectRoi.width - 1, rectRoi.y + rectRoi.height - 1),
					  cv::Scalar(255, 0, 0), 2);


        const std::vector<int> &labels           = pupilTracker->getClusterLabels();
        const gt::Clusteriser &clusteriser       = pupilTracker->getClusteriser();
        const std::vector<gt::Cluster> &clusters = clusteriser.getClusters();
        const std::vector<int> &indClusters      = clusteriser.getClusterIndices();

        for(size_t i = 0; i < indClusters.size(); ++i) {

            char txt[128];
            sprintf(txt, "%d", labels[i]);
            cv::putText(d,
                        txt,
                        clusters[indClusters[i]][0],
                        cv::FONT_HERSHEY_SIMPLEX,
                        0.5,
                        cv::Scalar(255, 0, 200),
                        1,
                        CV_AA);

        }

	}


	if(bTrackOk) {

		/*************************************************************
		 * Track image
		 *************************************************************/

		cv::Rect roi = cv::Rect(w + border_w, h + border_w, w, h);
		cv::Mat d(img_display, roi);


		const cv::Rect &rectRoi = pupilTracker->getROI();

		const cv::Mat &imgGray = pupilTracker->getGrayImage();
		const cv::Mat imgGrayRoi = cv::Mat(imgGray, rectRoi);

		cv::Mat imgDest = cv::Mat(d, rectRoi);

		cv::cvtColor(imgGrayRoi, imgDest, CV_GRAY2BGR);

		const std::vector<cv::Point> &cluster_pupil = pupilTracker->getClusterPupil();
		if(cluster_pupil.size() != 0) {

			std::vector<std::vector<cv::Point> > cluster(1);
			cluster[0] = cluster_pupil;

			cv::drawContours(d, cluster, -1, cv::Scalar(0, 105, 0), 2, CV_AA);

			cv::rectangle(d,
						  cv::Point(rectRoi.x, rectRoi.y),
						  cv::Point(rectRoi.x+rectRoi.width-1, rectRoi.y+rectRoi.height-1),
						  cv::Scalar(255, 0, 0), 2);

			const std::vector<cv::Point2d> &crs = pupilTracker->getCornealReflections();
			if(crs.size()) {
				if(crs[0].x != -1) {
					for(int i = 0; i < (int)crs.size(); ++i) {
						int x = (int)(crs[i].x + 0.5);
						int y = (int)(crs[i].y + 0.5);
						int x1 = x - 5;
						int x2 = x + 5;
						int y1 = y - 5;
						int y2 = y + 5;
						cv::line(d, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0), 2);
						cv::line(d, cv::Point(x2, y1), cv::Point(x1, y2), cv::Scalar(0, 255, 0), 2);
					}
				}
			}


			const cv::RotatedRect *ellipse = pupilTracker->getEllipsePupil();

			// draw a cross at the centre of the ellipse
			int x	= (int)(ellipse->center.x + 0.5);
			int y	= (int)(ellipse->center.y + 0.5);
			int x1	= x - 5;
			int x2	= x + 5;
			int y1	= y - 5;
			int y2	= y + 5;
			cv::line(d, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 0, 255), 1);
			cv::line(d, cv::Point(x2, y1), cv::Point(x1, y2), cv::Scalar(0, 0, 255), 1);

			cv::ellipse(d, *ellipse, cv::Scalar(0, 0, 255), 2, CV_AA);


            if(elements.size() > 1) {

                /*********************************************************************
                 * Draw the glint labels
                 *********************************************************************/
                for(size_t i = 0; i < elements.size(); ++i) {

                    const group::Element &el = elements[i];

                    char txt[10];
                    sprintf(txt, "%d", el.label);

                    cv::putText(d, std::string(txt), el.p,
                                cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 255), 2);

                }

            }

		}

	} else {

		cv::Rect roi = cv::Rect(w + border_w, h + border_w, w, h);

		cv::Mat d(img_display, roi);

        cv::Mat(img_orig_bgr, cv::Rect(0, 0, w, h)).copyTo(d);

        cv::putText(d,
                    "Track failed",
                    cv::Point(50, 50),
                    cv::FONT_HERSHEY_SIMPLEX,
                    0.8,
                    cv::Scalar(0, 0, 255),
                    2,
                    CV_AA);

	}


	cv::line(img_display, cv::Point(w+border_w/2, 0), cv::Point(w+border_w/2, 2*h), cv::Scalar(0, 105, 255), 5);
	cv::line(img_display, cv::Point(0, h+border_w/2), cv::Point(2*w, h+border_w/2), cv::Scalar(0, 105, 255), 5);

}


void resize_images() {
	cv::Size sz = img_frame.size();

	img_gray.release();
	img_gray	= cv::Mat(sz, CV_8UC1);
}


void handleKey(int fkey) {

	char key = (char)fkey;


	bool specialKey = false;
	if((fkey & 0xff00) == 0xff00) {
		specialKey = true;
	}

	bool toSettingsPanel = false;

	// Handle KEY_DOWN and KEY_UP in settings window
	if((fkey & 0xffff) == KEY_UP || (fkey & 0xffff) == KEY_DOWN) {
		toSettingsPanel = true;
	}

	if((!specialKey && (key >= 'A' && key <= 'Z')) ||
		key == '_' || key == '\b' ||
		key == '\n') {

		toSettingsPanel = true;
	}

	// Handle some characters in settings window
	if(toSettingsPanel) {

		settingsPanel.keyboardEvent(fkey);
		return;
	}

	// Handle other keys here.

	switch(key) {
		case 27: {
			prog_state.running = false;
			break;
		}

		case 'c': {
			img_scene = cv::Mat::zeros(SCENE_H, SCENE_W, CV_8UC3);
			calibrationPoints.clear();
			prog_state.map2scene = false;
			break;
		}

		case 'o': {
			prog_state.output_values = !prog_state.output_values;
			break;
		}

		case 'q': {
			prog_state.running = false;
			break;
		}

		case 'p': {
			prog_state.paused = !prog_state.paused;
			break;
		}

		case 'r': {
			pupilTracker->reset();
			break;
		}

		case '+': {
			pupilTracker->inc_nof_crs(1);
			break;
		}

		case '-': {
			pupilTracker->inc_nof_crs(-1);
			break;
		}

        case 'l': {

            tldTracker->learningEnabled = !tldTracker->learningEnabled;
            printf("TLD learning %d\n", tldTracker->learningEnabled);
            break;
        }

        case 'a': {
            tldTracker->alternating = !tldTracker->alternating;
            printf("TLD alternating %d\n", tldTracker->alternating);
            break;
        }

		case 83: { // right arrow
			if(prog_state.paused) {

				int len = cap.get(CV_CAP_PROP_POS_FRAMES);
				int newpos = std::max(cap.get(CV_CAP_PROP_POS_FRAMES), (double)len);

				cap.set(CV_CAP_PROP_POS_FRAMES, newpos);
				prog_state.change_frame = true;
			}
			else {
				int newpos = std::min(cap.get(CV_CAP_PROP_POS_FRAMES) + 20, cap.get(CV_CAP_PROP_FRAME_COUNT));
				cap.set(CV_CAP_PROP_POS_FRAMES, newpos);
			}
			break;
		}

		case 81: { // left arrow

			if(prog_state.paused) {
				int newpos = std::max(cap.get(CV_CAP_PROP_POS_FRAMES) - 2, 0.0);
				cap.set(CV_CAP_PROP_POS_FRAMES, newpos);
				prog_state.change_frame = true;
			}
			else {
				int newpos = std::max(cap.get(CV_CAP_PROP_POS_FRAMES) - 20, 0.0);
				cap.set(CV_CAP_PROP_POS_FRAMES, newpos);
			}

			break;
		}

		case 82: { // up arrow
			break;
		}

		case 84: { // down arrow

			break;
		}

		default:
			break;
	}

}


void rotate_line(cv::Point &pt1, cv::Point &pt2, float ang) {

	int max_x = std::max(pt1.x, pt2.x);
	int max_y = std::max(pt1.y, pt2.y);
	int len_x = std::abs(pt1.x - pt2.x);
	int len_y = std::abs(pt1.y - pt2.y);


	int centerx = max_x-len_x/2;
	int centery = max_y-len_y/2;

	double R[4] = {cos(ang), -sin(ang), sin(ang), cos(ang)};

	{
	double x = pt1.x-centerx;
	double y = pt1.y-centery;
	double tmpx = R[0]*x+R[1]*y;
	double tmpy = R[2]*x+R[3]*y;

	pt1.x = centerx+tmpx;
	pt1.y = centery+tmpy;
	}

	{
	double x = pt2.x-centerx;
	double y = pt2.y-centery;
	double tmpx = R[0]*x+R[1]*y;
	double tmpy = R[2]*x+R[3]*y;

	pt2.x = centerx+tmpx;
	pt2.y = centery+tmpy;
	}

}


void Draw_Cross(cv::Mat &image,
                int centerx,
                int centery,
                int x_cross_length,
                int y_cross_length,
                cv::Scalar color,
                int lineWidth)
{

    static float ang = 0;
    ang+= PI/10.0;

	cv::Point pt1,pt2,pt3,pt4;

	pt1.x = centerx - x_cross_length;
	pt1.y = centery;
	pt2.x = centerx + x_cross_length;
	pt2.y = centery;

	rotate_line(pt1, pt2, ang);

	pt3.x = centerx;
	pt3.y = centery - y_cross_length;
	pt4.x = centerx;
	pt4.y = centery + y_cross_length;

	rotate_line(pt3, pt4, ang);

	cv::line(image,pt1,pt2,color,lineWidth, 8);
	cv::line(image,pt3,pt4,color,lineWidth, 8);
}


void mouse_callback_scene( int event, int x, int y, int, void *) {

	switch(event) {
		case CV_EVENT_LBUTTONDOWN: {

			// Is a pupil available?
			const std::vector<cv::Point> &cluster_pupil = pupilTracker->getClusterPupil();
			if(cluster_pupil.size() == 0) {
				break;
			}

			// Is a CR available?
			const cv::RotatedRect *ellipse = pupilTracker->getEllipsePupil();
			const std::vector<cv::Point2d> &crs = pupilTracker->getCornealReflections();
			cv::Point2f cr = crs[0];
			if(cr.x == -1) {
				break;
			}

			// Is calibration already complete?
			if(prog_state.map2scene) {
				break;
			}

			// Are there already sufficient amount of calibration points?
			if(calibrationPoints.size() >= 9) {
				break;
			}

			// Calculate pupil centre	
			cv::Point2f pupil(ellipse->center.x + ellipse->size.width / 2,
					ellipse->center.y + ellipse->size.height / 2);

			calibrationPoints.push_back(CalibrationPoint(pupil, cr, cv::Point2f(x, y)));

			// Plot
			Draw_Cross(img_scene, x, y, 10, 10, cv::Scalar(255, 255, 255, 255), 1);

			// Store the pupil image
			std::stringstream strs;
			strs << "./img/img_display" << calibrationPoints.size() << ".png";
			cv::imwrite(strs.str(), img_display);

			
			break;
		}

		// Calibration is enabled by pressing the right button
		case CV_EVENT_RBUTTONDOWN: {
			sceneTracker.calibrate(calibrationPoints);
			prog_state.map2scene = true;
			break;
		}

		// Screen can be cleared by the middle button
		case CV_EVENT_MBUTTONDOWN: {
			img_scene = cv::Mat::zeros(SCENE_H, SCENE_W, CV_8UC3);
			break;
		}
	}
}


void mouse_callback( int event, int x, int y, int, void *) {
	switch(event) {
		case CV_EVENT_LBUTTONDOWN: {
			if(x < img_gray.cols && y < img_gray.rows) {
/*				const Starburst &starburst = tracker->getStarburstObject();
				starburst.setStartPoint(cv::Point2f(x, y));
*/
			}
			break;
		}
	}
}

