#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <stdio.h>

#include "GazeTracker.h"
#include "SceneMapper.h"
#include "trackerSettings.h"
#include "settingsIO.h"
#include "localTrackerSettings.h"
#include "settingsPanel.h"

#include "VideoHandler.h"


/******************************************************************************
 * Video Configuration
 *****************************************************************************/

static const int W_FRAME	= 640;
static const int H_FRAME	= 480;
static const int FRAMERATE	= 30;


using namespace std;

/******************************************************************************
 * A class for storing last N samples
 *****************************************************************************/

static const int CAPACITY = 15; 
class SampleContainer {
	public:
		SampleContainer() {
			pos = sz = 0;
			samples.resize(CAPACITY);
		}

		void add(cv::Point p) {
			if(sz != CAPACITY) {
				++sz;
			}

			samples[pos] = p;

			pos = (pos + 1) % CAPACITY;


		}

		const std::vector<cv::Point> &getSamples(int &n) {n = sz; return samples;} 


	private:
		int pos;
		int sz;

		std::vector<cv::Point> samples;

};


/******************************************************************************
 * Global variables
 *****************************************************************************/

static SampleContainer sample_container;
static gt::GazeTracker* tracker;
static Camera* eye_cam;
static Camera* scene_cam;
static gt::LED_Pattern* led_pattern;
static SceneMapper* mapper;

static bool b_paused = false;
static bool b_running = true;

SettingsPanel settingsPanel;

/* Variable for holding the difference between the real point and the point
 * from the gaze tracker */

static cv::Point2d calibrationDifference;

/******************************************************************************
 * Calibration is made here. When the user clicks some point in the scene
 * image, the difference between the currently mapped point and the clicked
 * point is stored. The difference is used to compensate the offset.
 * NOTE! After figuring out all possible error sources, this should be
 * replaced by kappa correction
 *****************************************************************************/

static void mouse_callback( int event, int x, int y, int flags, void* param )
{

	// Check that the mouse have been clicked
	if(event != CV_EVENT_LBUTTONDOWN) {
		return;
	}

	const double *c_pupil = tracker->getCentrePupil();
	const double *c_cornea = tracker->getCentreCornea();

	// Check that the gaze and the pupil vectors are available

	if(c_cornea[0] == c_cornea[0] &&
		c_cornea[1] == c_cornea[1] &&
		c_cornea[2] == c_cornea[2] &&
		c_pupil[0] == c_pupil[0] &&
		c_pupil[1] == c_pupil[1] &&
		c_pupil[2] == c_pupil[2] &&
		c_pupil[0] != 0 &&
		c_pupil[1] != 0 &&
		c_pupil[2] != 0 &&
		c_cornea[0] != 0 &&
		c_cornea[1] != 0 &&
		c_cornea[2] != 0) {

		// Compute the POG from with the gaze
		Eigen::Vector3d eig_p_cornea(c_cornea[0], c_cornea[1], c_cornea[2]);
		Eigen::Vector3d eig_p_pupil(c_pupil[0], c_pupil[1], c_pupil[2]);
		cv::Point2d point;
		mapper->getPosition(eig_p_cornea, eig_p_pupil, point);

		// Compute the difference
		calibrationDifference = cv::Point2d(x, y) - point;

	}
}


/******************************************************************************
 * Draw tracker results
 *****************************************************************************/

void drawResultImages(cv::Mat& res_eye_frame, cv::Mat& res_scene_frame)
{

	/**********************************************************************
	 * Draw results from pupil tracking
	 *********************************************************************/

	gt::PupilTracker *pupil_tracker = tracker->getPupilTracker();
	const std::vector<cv::Point> *cluster_pupil = pupil_tracker->getClusterPupil();

	if(cluster_pupil != NULL) {
		std::vector<std::vector<cv::Point> > cluster(1); cluster[0] = *cluster_pupil;
		cv::drawContours(res_eye_frame, cluster, -1, cv::Scalar(0, 0, 255), 1, CV_AA);
	}


	gt::PupilTracker *pupilTracker = tracker->getPupilTracker();
	const cv::Rect &rect_roi = pupilTracker->getROI();
	cv::rectangle(res_eye_frame,
				  cv::Point(rect_roi.x, rect_roi.y),
				  cv::Point(rect_roi.x+rect_roi.width-1, rect_roi.y+rect_roi.height-1),
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
				cv::line(res_eye_frame, cv::Point(x1, y1), cv::Point(x2, y2), cv::Scalar(0, 255, 0), 2);
				cv::line(res_eye_frame, cv::Point(x2, y1), cv::Point(x1, y2), cv::Scalar(0, 255, 0), 2);
			}
		}
	}

	/**********************************************************************
	 * Draw the previous gaze points to the scene image
	 *********************************************************************/

	int sample_count = 0;
	const std::vector<cv::Point>& samples = sample_container.getSamples(sample_count);

	for(int i = 0; i < sample_count; i++) {

		const cv::Point &point = samples[i];
		cv::circle(res_scene_frame, point, 10, cv::Scalar(255,0,0), 1, CV_AA);

/*		if (i != 0) {
			const cv::Point &p1 = samples[i-1];
			const cv::Point &p2 = samples[i];
			//cv::line(res_scene_frame, p1, p2, cv::Scalar(0, 255, 0), 3);
		}
*/
	}


	/**********************************************************************
	 * Gaze vector drawing
	 *********************************************************************/

	const double *c_pupil = tracker->getCentrePupil();
	const double *c_cornea = tracker->getCentreCornea();

	/*
	 * Sanity checks. A float or a double is NaN when
	 * d != d == true.
	 */
	if(c_cornea[0] == c_cornea[0] &&
		c_cornea[1] == c_cornea[1] &&
		c_cornea[2] == c_cornea[2] &&
		c_pupil[0] == c_pupil[0] &&
		c_pupil[1] == c_pupil[1] &&
		c_pupil[2] == c_pupil[2] &&
		c_pupil[0] != 0 &&
		c_pupil[1] != 0 &&
		c_pupil[2] != 0 &&
		c_cornea[0] != 0 &&
		c_cornea[1] != 0 &&
		c_cornea[2] != 0) {

		const Camera * cam = tracker->getCamera();

		cv::Point3d p_cornea(c_cornea[0], c_cornea[1], c_cornea[2]);

		double u1, v1;
		cam->worldToPix(p_cornea, &u1, &v1);

		cv::Point3d p_pupil(c_pupil[0], c_pupil[1], c_pupil[2]);

		double u2, v2;
		cam->worldToPix(p_pupil, &u2, &v2);

		cv::Point3d pupil_to_cornea(	p_pupil.x - p_cornea.x,
						p_pupil.y - p_cornea.y,
						p_pupil.z - p_cornea.z);

		pupil_to_cornea *= 3.0;
		cam->worldToPix(pupil_to_cornea + p_cornea, &u2, &v2);

		/***************************************************************
		 * Draw the gaze vector
		 **************************************************************/

		cv::line(res_eye_frame, cv::Point2d((int)(u1 + 0.5),
			(int)(v1 + 0.5)), cv::Point2d((int)(u2 + 0.5),
			(int)(v2 + 0.5)), cv::Scalar(0, 0, 255), 1, 0);

		/***************************************************************
		 * Map the gaze vector to the scene
		 **************************************************************/

		cv::Point2d point;
		Eigen::Vector3d eig_p_cornea(c_cornea[0], c_cornea[1], c_cornea[2]);
		Eigen::Vector3d eig_p_pupil(c_pupil[0], c_pupil[1], c_pupil[2]);

		// Map the gaze to the scene
		mapper->getPosition(eig_p_cornea, eig_p_pupil, point);

		// Apply "calibration"
		point += calibrationDifference;

		sample_container.add(point);
		cv::circle(res_scene_frame, point, 10, cv::Scalar(0,0,255), 2, CV_AA);


	}

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

	if(!specialKey && (key >= 'A' && key <= 'Z') ||
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
			b_running = false;
			break;
		}

		case 'q': {
			b_running = false;
			break;
		}

		case 'p': {
			b_paused = !b_paused;
			break;
		}

		case 'r': {
			tracker->reset();
			break;
		}

		case '+': {
			gt::PupilTracker *pupil_tracker = tracker->getPupilTracker();
			pupil_tracker->inc_nof_crs(1);
			break;
		}

		case '-': {
			gt::PupilTracker *pupil_tracker = tracker->getPupilTracker();
			pupil_tracker->inc_nof_crs(-1);
			break;
		}

		default:
			break;
	}

}

/*******************************************************************************
 * main()
 ******************************************************************************/

int main (int argc, char **argv)
{

	/**********************************************************************
	 * Initialize the main window
	 *********************************************************************/

	cv::Mat scene_frame, eye_frame, img_gray;
	cv::namedWindow ("Eye Camera", CV_WINDOW_AUTOSIZE);
	cv::namedWindow ("Scene Camera", CV_WINDOW_AUTOSIZE);
	cvSetMouseCallback("Scene Camera", mouse_callback, NULL);

	/**********************************************************************
	 * Initialize the settings panel
	 *********************************************************************/

	// Read settings from file
	{
		SettingsIO settingsFile("default.xml");
		LocalTrackerSettings localSettings;
		localSettings.open(settingsFile);
		settingsPanel.setSettings(localSettings);
		settingsPanel.open(settingsFile);
	}

	settingsPanel.show();

	/**********************************************************************
	 * Initialize the video streams
	 *********************************************************************/

	gst_init(0, NULL);

	cv::VideoCapture eye_cap, scene_cap;

	std::vector<std::string> devnames(2);

	for(int i = 0; i < 2; ++i) {

		char tmp[64];
		sprintf(tmp, "%s%d", "video", i+1);

		devnames[i] = std::string(tmp);
	}


	VideoHandler *videos = new VideoHandler(devnames);
	if(!videos->init(W_FRAME, H_FRAME, FRAMERATE, FORMAT_MJPG)) {
		std::cout << "Could not initialise the video handler" << std::endl;
		return -1;
	}

	/**********************************************************************
	 * Initialize the camera variables
	 *********************************************************************/

	// Initialize the eye camera	
	{
		eye_cam = new Camera();
//		double intr_mat[9] = {624.336580,0.000000,307.748979,0.000000,625.094574,241.744636,0.000000,0.000000,1.000000};
		double intr_mat[9] = {624.336580,0.0,0.0,0.0,625.094574,0.0,307.748979, 241.744636,1.000000};
		double dist_mat[5] = {0.010988,0.278925,0.000626,0.002640,-0.928955};
		eye_cam->setIntrinsicMatrix(intr_mat);
		eye_cam->setDistortion(dist_mat);
	}

	// Initialize the scene camera
	{
		scene_cam = new Camera();
//		double intr_mat[9] = {600.4577719872011, 0.000000000000000, 346.6058023899264, 0.000000000000000, 600.3870054985101, 225.5436246998888, 0.000000000000000, 0.000000000000000, 1.000000000000000};
		double intr_mat[9] = {600.4577719872011, 0.0, 0.0, 0.0, 600.3870054985101, 0.0, 346.6058023899264, 225.5436246998888, 1.0};

		double dist_mat[5] = {0.03672340168667670, -0.04230613123096214, 0.002254662790009281, 0.004653749632542676, -0.06320079390269241};
		scene_cam->setIntrinsicMatrix(intr_mat);
		scene_cam->setDistortion(dist_mat);
	}

	/**********************************************************************
	 * Setup mapping to the scene
	 *********************************************************************/

	Eigen::Matrix4d A;
	A << -0.7819930767407347, 0.2950909546850828, 0.5490065176230658, -1.565028954437844, -0.05243528464052402, 0.8465556648028852, -0.5297112867542206, -1.875917936202043, -0.6210775868232781, -0.4430178719342949, -0.6465274907477501, 0.8702844663922811, 0.000000000000000, 0.000000000000000, 0.000000000000000, 1.000000000000000;
	A = A.inverse();
	mapper = new SceneMapper(A, scene_cam, 1000);

	/**********************************************************************
	 * Create the eye tracker
	 *********************************************************************/

	tracker = new gt::GazeTracker(eye_cam);

	std::vector<Eigen::Vector3d> positions(6);
	positions[0] = Eigen::Vector3d(0.222593,-16.073684,49.863236);
	positions[1] = Eigen::Vector3d(-18.273366,-9.475727,23.202178);
	positions[2] = Eigen::Vector3d(-14.026665,-1.464826,21.947697);
	positions[3] = Eigen::Vector3d(13.252075,14.089948,32.273235);
	positions[4] = Eigen::Vector3d(20.036881,2.891225,50.435503);
	positions[5] = Eigen::Vector3d(17.142129,-5.991405,54.775926);
	
	led_pattern = new gt::SixLEDs(positions);

	tracker->init(led_pattern);
	
	gt::PupilTracker *pupil_tracker = tracker->getPupilTracker();

	pupil_tracker->set_nof_crs(6);

	// Reset the calibration
	calibrationDifference = cv::Point2d(0, 0);

	/**********************************************************************
	 * Main loop
	 *********************************************************************/

	b_running = true;
	while(b_running) {

		trackerSettings.set(*settingsPanel.getSettings());

		/**************************************************************
		 * Try capturing the frames
		 *************************************************************/

		std::vector<CameraFrame *> imgs(2);

		// try get the video frames
		if(videos->getFrames(imgs)) {

			CameraFrame *eye_img = imgs[0];

			cv::Mat eye_frame(eye_img->h,
					eye_img->w,
					CV_8UC3,
					eye_img->data,
					eye_img->w * eye_img->bpp);

			CameraFrame *scene_img = imgs[1];

			cv::Mat scene_frame(scene_img->h,
					scene_img->w,
					CV_8UC3,
					scene_img->data,
					scene_img->w * scene_img->bpp);

			cv::cvtColor(scene_frame, scene_frame, CV_RGB2BGR);


			/******************************************************
			 * Analyze a frame
			 *****************************************************/

			cv::flip(eye_frame, eye_frame, 1);
			cv::cvtColor(eye_frame, img_gray, CV_RGB2GRAY);
			tracker->track(&img_gray);

			/******************************************************
			 * Draw the results
			 *****************************************************/

			cv::Mat res_eye_frame = eye_frame.clone();
			cv::Mat res_scene_frame = scene_frame.clone();
			drawResultImages(res_eye_frame, res_scene_frame);

			imshow("Eye Camera", res_eye_frame);
			imshow("Scene Camera", res_scene_frame);

			// the caller of VideoHandler::getFrames() must release the frames
			delete eye_img;
			delete scene_img;

		}


		/**************************************************************
		 * Keyboard handling
		 *************************************************************/

		int key = cv::waitKey(2);

		if(key > 0) {
			handleKey(key);
		}

	}

	// Store the settings to file
	{
		SettingsIO settingsFile;
		LocalTrackerSettings localSettings;
		localSettings = *settingsPanel.getSettings();
		localSettings.save(settingsFile);
		settingsPanel.save(settingsFile);
		settingsFile.save("default.xml");
	}

	delete tracker;
	delete led_pattern;
	delete eye_cam;
	delete scene_cam;
	delete videos;

	pthread_exit(NULL);

	return 0;
}

