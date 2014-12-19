#include "VideoHandler.h"
#include "DualFrameReceiver.h"
#include "Settings.h"
#include "trackerSettings.h"
#include <ctime>


/* Extern the global tracker protecting mutex */
extern pthread_mutex_t mutex_tracker;


/*********************************************************
 * Prototypes
 *********************************************************/
static void main_loop();
static bool init_all(const char *input_file);
static bool init_video(const Settings &settings);
static void quit();
static void print_usage_info();



/*********************************************************
 * Globals
 *********************************************************/

/* Stream related */
static const int NDEVS						= 2;
static const int FRAMERATE					= 30;
static VideoHandler *videos					= NULL;
static DualFrameReceiver *receiver			= NULL;


/* Other */
static const int W_FRAME1					= 640;
static const int H_FRAME1					= 480;
static const int W_FRAME2					= 640;
static const int H_FRAME2					= 480;



int main(int nof_args, const char **args) {

	/******************************************
	 * check args
	 ******************************************/
	if(nof_args < 2) {

		print_usage_info();

		return EXIT_FAILURE;
	}

	/******************************************
	 * initialise everything
	 ******************************************/
	if(!init_all(args[1])) {

		quit();

		return EXIT_FAILURE;
	}


	/******************************************
	 * run the main loop
	 ******************************************/
	main_loop();


	/******************************************
	 * clean up
	 ******************************************/
	quit();


	/******************************************
	 * exit the main thread
	 ******************************************/
	pthread_exit(NULL);


	/******************************************
	 * return success
	 ******************************************/
	return EXIT_SUCCESS;

}


void main_loop() {

	/***********************************************************
	 *  The main loop
	 ***********************************************************/
	while(getchar() != 'q');

}




bool init_all(const char *input_file) {

	/***********************************************************
	 * Init gstreamer
	 ***********************************************************/
	gst_init(0, NULL);


	/***********************************************************
	 * Read the settings
	 ***********************************************************/
	Settings settings;
	if(!settings.readSettings(input_file)) {

		printf("main(): Could not read settings\n");

		return false;
	}


	/**************************************************
	 * Read the tracker settings
	 **************************************************/
	{
		SettingsIO settingsFile(settings.gazetrackerFile);
		LocalTrackerSettings localSettings;
		localSettings.open(settingsFile);
		trackerSettings.set(localSettings);
	}


	/***********************************************************
	 * Initialise the video streams
	 ***********************************************************/
	if(!init_video(settings)) {

		return false;
	}

	return true;

}


bool init_video(const Settings &settings) {

	std::vector<VideoInfo> info(NDEVS);
	std::vector<std::string> camera_save_paths(NDEVS);


	// Video 1
	info[0].devname	= settings.dev1;
	info[0].w		= W_FRAME1;
	info[0].h		= H_FRAME1;
	info[0].fps		= FRAMERATE;
	info[0].format	= FORMAT_MJPG;


	// Video 2
	info[1].devname	= settings.dev2;
	info[1].w		= W_FRAME2;
	info[1].h		= H_FRAME2;
	info[1].fps		= FRAMERATE;
	info[1].format	= FORMAT_MJPG;


	/*
	 * It is important to create receiver before videos, because the receiver
	 * must be ready before creating the streams.
	 */
	receiver = new DualFrameReceiver();
	if(!receiver->init(settings.oput_dir, // UGLY: this is now the socket file, TODO: Change to non-ugly
					   settings.eyeCamCalibFile,
					   settings.sceneCamCalibFile,
					   settings.mapperFile)) {

		return false;
	}

	videos = new VideoHandler();
	if(!videos->init(info, receiver)) {
		std::cout << "main(): Could not initialise the video handler" << std::endl;
		return false;
	}


	return true;

}


void print_usage_info() {

	printf(
		"Usage:\n"
		"  ./gazetoworld <settingsfile>.xml\n"
	);

}


void quit() {

	/*
	 * It is important to delete videos before receiver, because the that way
	 * the videostreamer will not be forwarding frames to the receiver after
	 * the receiver has been deallocated.
	 */
	delete videos;
	delete receiver;

}

