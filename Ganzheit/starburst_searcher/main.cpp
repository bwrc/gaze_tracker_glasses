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


static const double PI = 3.14;


using namespace std;



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

            fontScale *= 0.98;

        }

    }

private:

    int count;
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



static const int SCENE_W = 1024;
static const int SCENE_H = 768;

void drawResulImages(bool starburstFailed, const cv::Mat &img_orig_bgr, const cv::Mat &img_binary, cv::Mat &img_display, double runningTime);
void Draw_Cross(cv::Mat &image, int centerx, int centery, int x_cross_length, int y_cross_length, cv::Scalar color);
void handleKey(int key);
void mouse_callback(int event, int x, int y, int flags, void* param);
void mouse_callback_scene(int event, int x, int y, int flags, void* param);
void resize_images();


ProgramState prog_state;

gt::PupilTracker* tracker;
SceneTracker sceneTracker;
SettingsPanel settingsPanel;

unsigned char th = 0;
cv::VideoCapture cap;

cv::Mat img_frame;
cv::Mat img_display;
cv::Mat img_gray;
cv::Mat img_scene;

std::vector<CalibrationPoint> calibrationPoints;


TextAnimator textAnimator;


void drawStarburst(cv::Mat & bgr_frame, const pointContainer &inliers, const pointContainer & outliers)
{

	cv::rectangle(bgr_frame, cv::Point(0, 0), cv::Point(bgr_frame.cols-1, bgr_frame.rows-1), cv::Scalar(0, 0, 0), CV_FILLED);

	int step = bgr_frame.step;


	if(bgr_frame.rows <= 0 || bgr_frame.cols <= 0) {
		cout << "drawStarburst() called with empty frame!\n";
		exit(1);
	}

	if (inliers.size()) {

		pointContainer::const_iterator edge;
		for (edge = inliers.begin();
			edge != inliers.end( ) ;
			edge++ ) {

			int ind = round(edge->point.y) * step + 3 * round(edge->point.x);
			if(round(edge->point.y) > bgr_frame.rows ||
				round(edge->point.x) > bgr_frame.cols) {
				cout << "Bad inlier point!\n";
				exit(1);
			}
			bgr_frame.data[ind]	= 0;
			bgr_frame.data[ind+1]	= 255;
			bgr_frame.data[ind+2]	= 0;
		}
	}

	if (outliers.size()) {

		pointContainer::const_iterator edge;
		for(edge = outliers.begin();
			edge != outliers.end() ;
			edge++ ) {

			int ind = round(edge->point.y) * step + 3 * round(edge->point.x);
			if(round(edge->point.y) > bgr_frame.rows ||
				round(edge->point.x) > bgr_frame.cols) {
				cout << "Bad outlier point!\n";
				exit(1);
			}
			bgr_frame.data[ind]	= 0;
			bgr_frame.data[ind+1]	= 0;
			bgr_frame.data[ind+2]	= 255;
		}
	}
}


void outputPointValues(cv::Mat &gray, const pointContainer & edge_points) {

	if (edge_points.size()) {

		int step = gray.step;

		cout << "Pixel values: ";

		pointContainer::const_iterator edge;
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

		std::cout << "Usage: " << std::endl << "    " << std::string(argv[0]) <<
			" videofile [--track] [--config=filename.xml]" << std::endl <<
			" Or " << std::endl << "    " << std::string(argv[0]) << 
			" cam=<number of device> [--track] [--config=filename.xml]" <<
			std::endl;

		return -1;
	}

	MyTimer timer;

    // parsed from command line arguments
	std::string configurationFilename("default.xml");
	std::string videoFilename;
    std::string strTrackerTh;

	img_scene = cv::Mat::zeros(SCENE_H, SCENE_W, CV_8UC3);

	/**********************************************************************
	 * Initialize the main win
	 *********************************************************************/
	cv::namedWindow ("Main", CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback("Main", mouse_callback, NULL);


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
		} else if(setting == std::string("--threshold")) {
			strTrackerTh = value;
		} 
        else if(setting == std::string("cam")) {
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
	 * Initialize the settings panel
	 *********************************************************************/
	// Read settings from file
	{
		SettingsIO settingsFile(configurationFilename);
		LocalTrackerSettings localSettings;
		localSettings.open(settingsFile);
		settingsPanel.setSettings(localSettings);
		settingsPanel.open(settingsFile);
	}

	settingsPanel.show();




	/**********************************************************************
	 * Initialize the pupil tracker
	 *********************************************************************/

	tracker = new gt::PupilTracker();
	tracker->set_nof_crs(6);

    if(!strTrackerTh.empty()) {

        if(strTrackerTh == "auto") {
            tracker->useAutoTh(true);
        }
        else {
            tracker->useAutoTh(false);

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

        // because gazetoworld does it as well. Comparison is more consistent.
        //cv::flip(img_frame, img_frame, 1);

		cur_sz = img_frame.size();
		if(cur_sz != old_sz) {
			resize_images();
		}
		old_sz = cur_sz;

		cv::cvtColor(img_frame, img_gray, CV_BGR2GRAY);


        /*
         * Had to comment this away, because the corneal refelctions were
         * not found as well anymore.
         */
		// apply Gaussian blur to reduce noise
        //		cv::GaussianBlur(img_gray, img_gray, cv::Size(5, 5), 0);

		timer.markTime();
		tracker->track(img_gray);

		double runningTime = timer.getElapsed_micros();
		//cout << "Time: " << runningTime / 1000.0 << "ms" <<endl;
        //cout << runningTime << endl;


		const Starburst &starburst = tracker->getStarburstObject();
		cv::Point2f center;
		center = starburst.getROICenter();

		bool starburstFailed = false;
		if(center.x != -1 && center.y != -1) {

			th = starburst.getThreshold();

			// Draw and ..


			if(prog_state.output_values) {
				const pointContainer &edge_points = starburst.getInlierPoints();

				outputPointValues(img_gray, edge_points);
			}

			if(prog_state.track) {

				if(prog_state.map2scene) {

					// Show the gaze point on the screen
					const cv::RotatedRect *ellipse = tracker->getEllipsePupil();
					const std::vector<cv::Point2d> &crs = tracker->getCornealReflections();
					cv::Point2f cr = crs[0];
					cv::Point2f pupil(ellipse->center.x + ellipse->size.width / 2,
							ellipse->center.y + ellipse->size.height / 2);

					cv::Point2f gaze_point = sceneTracker.track(pupil, cr);

					printf("Gaze coordinate: %.2f, %.2f\n", gaze_point.x, gaze_point.y);
					Draw_Cross(img_scene, gaze_point.x, gaze_point.y, 5, 5, cv::Scalar(0, 0, 255, 255));

				}

				imshow("Scene Window", img_scene);
			}

		} else {
			starburstFailed = true;
		}

		// .. show
		const cv::Mat &img_binary = tracker->getBinaryImage();
		drawResulImages(starburstFailed, img_frame, img_binary, img_display, runningTime);

        textAnimator.update(img_display);

		imshow("Main", img_display);

	}


/*
	// Store the settings to file
	{
		SettingsIO settingsFile;
		LocalTrackerSettings localSettings;
		localSettings = *settingsPanel.getSettings();
		localSettings.save(settingsFile);
		settingsPanel.save(settingsFile);
		settingsFile.save(configurationFilename);
	}
*/

	delete tracker;

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
void drawResulImages(bool starburstFailed,
					 const cv::Mat &img_orig_bgr,
					 const cv::Mat &img_binary,
					 cv::Mat &img_display,
					 double runningTime) {

	int w = img_orig_bgr.cols;
	int h = img_orig_bgr.rows;

	static const int border_w = 5; // please let it be odd
	int dw = 2*w + border_w;
	int dh = 2*h + border_w;

	if(img_display.cols != dw || img_display.rows != dh) {
		img_display = cv::Mat::zeros(dh, dw, CV_8UC3);
	}

	/**********************************************************************
	 * orig image
	 *********************************************************************/
	{
		cv::Rect roi(0, 0, w, h);
		cv::Mat d(img_display, roi);
		img_orig_bgr.copyTo(d);

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



		if(!starburstFailed) {

            cv::Point p1(trackerSettings.CROP_AREA_X,
                         trackerSettings.CROP_AREA_Y);

            cv::Point p2(trackerSettings.CROP_AREA_X + trackerSettings.CROP_AREA_W - 1,
                         trackerSettings.CROP_AREA_Y + trackerSettings.CROP_AREA_H - 1);

			cv::rectangle(d,
                          p1,
                          p2,
						  cv::Scalar(255, 0, 0),
						  3);


		}

	}


	/**************************************************************
	 * Starburt image
	 *************************************************************/
	{
		cv::Rect roi(w+border_w, 0, w, h);
		cv::Mat d(img_display, roi);

		const Starburst &starburst = tracker->getStarburstObject();
		drawStarburst(d, starburst.getInlierPoints(), starburst.getOutlierPoints());
	}



	/**************************************************************
	 * Binary image
	 *************************************************************/

	if(img_binary.cols > 0 && img_binary.rows > 0) {

		const cv::Rect &rect_roi = tracker->getROI();

		cv::Rect roi(0, h+border_w, w, h);
		cv::Mat d(img_display, roi);
		cv::rectangle(d, cv::Point(0, 0), cv::Point(w-1, h-1), cv::Scalar(0, 0, 0), CV_FILLED);

		cv::Mat d_roi = cv::Mat(d, rect_roi);
		cv::Mat img_binary_roi		= cv::Mat(img_binary, rect_roi);
		cv::Mat img_binary_bgr_roi;
		cv::cvtColor(img_binary_roi, img_binary_bgr_roi, CV_GRAY2BGR);
		img_binary_bgr_roi.copyTo(d_roi);

		cv::rectangle(d,
					  cv::Point(rect_roi.x, rect_roi.y),
					  cv::Point(rect_roi.x+rect_roi.width-1, rect_roi.y+rect_roi.height-1),
					  cv::Scalar(255, 0, 0), 2);


        const std::vector<int> &labels = tracker->getClusterLabels();
        const gt::Clusteriser &clusteriser = tracker->getClusteriser();
        const std::vector<gt::Cluster> &clusters = clusteriser.getClusters();
        const std::vector<int> &indClusters = clusteriser.getClusterIndices();
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


	if(!starburstFailed) {

		/*************************************************************
		 * Track image
		 *************************************************************/

		cv::Rect roi(w+border_w, h+border_w, w, h);
		cv::Mat d(img_display, roi);

		// paint it black
		cv::rectangle(d, cv::Point(0, 0), cv::Point(w-1, h-1), cv::Scalar(0, 0, 0), CV_FILLED);

		const cv::Rect &rectROI = tracker->getROI();

		const cv::Mat &imgGray = tracker->getGrayImage();
		const cv::Mat imgGrayROI = cv::Mat(imgGray, rectROI);

		cv::Mat imgDest = cv::Mat(d, rectROI);

		cv::cvtColor(imgGrayROI, imgDest, CV_GRAY2BGR);

		const std::vector<cv::Point> &cluster_pupil = tracker->getClusterPupil();
		if(cluster_pupil.size() != 0) {

			std::vector<std::vector<cv::Point> > cluster(1);
			cluster[0] = cluster_pupil;

			cv::drawContours(d, cluster, -1, cv::Scalar(0, 105, 0), 2, CV_AA);

			const cv::Rect &rect_roi = tracker->getROI();
			cv::rectangle(d,
						  cv::Point(rect_roi.x, rect_roi.y),
						  cv::Point(rect_roi.x+rect_roi.width-1, rect_roi.y+rect_roi.height-1),
						  cv::Scalar(255, 0, 0), 2);

			const std::vector<cv::Point2d> &crs = tracker->getCornealReflections();
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


			const cv::RotatedRect *ellipse = tracker->getEllipsePupil();

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

		}

        cv::RotatedRect ellSearch = tracker->getSearchEllipse();
        cv::ellipse(d, ellSearch, cv::Scalar(0, 0, 0), 2, CV_AA);


	} else {
			{
				cv::Rect roi(0, h+border_w, w, h);
				cv::Mat d(img_display, roi);
				d = cv::Mat::zeros(roi.height, roi.width, CV_8UC3);
			}

			{
				cv::Rect roi(w+border_w, h+border_w, w, h);
				cv::Mat d(img_display, roi);
				d = cv::Mat::zeros(roi.height, roi.width, CV_8UC3);
			}

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
			tracker->reset();
			break;
		}

		case '+': {
			tracker->inc_nof_crs(1);
			break;
		}

		case '-': {
			tracker->inc_nof_crs(-1);
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


void Draw_Cross(cv::Mat &image, int centerx, int centery, int x_cross_length, int y_cross_length, cv::Scalar color)
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

	cv::line(image,pt1,pt2,color,1,8);
	cv::line(image,pt3,pt4,color,1,8);
}


void mouse_callback_scene( int event, int x, int y, int flags, void* param ) {
	switch(event) {
		case CV_EVENT_LBUTTONDOWN: {

			// Is a pupil available?
			const std::vector<cv::Point> &cluster_pupil = tracker->getClusterPupil();
			if(cluster_pupil.size() == 0) {
				break;
			}

			// Is a CR available?
			const cv::RotatedRect *ellipse = tracker->getEllipsePupil();
			const std::vector<cv::Point2d> &crs = tracker->getCornealReflections();
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
			Draw_Cross(img_scene, x, y, 10, 10, cv::Scalar(255, 255, 255, 255));

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


void mouse_callback( int event, int x, int y, int flags, void* param ) {
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

