There are multiple applications for testing various parts of the algorithm and a main application which performs gaze tracking. The most useful ones are described here. Any other application is not supported anymore. They might work and they can be modified etc. but e.g. interface changes are not updated into those applications.


**********************************************************************
* starburst_searcher/starburst
**********************************************************************

This application opens up a window with four images in the following way.
______________
| img1 | img2 |
|______|______|
| img3 | img4 |
|______|______|

img1: The original BGR image with the Crop area highlighted
img2: The starburst results, inliers (green) and outliers (red)
img3: The binary image
img4: The track results, pupil ellipse, CRs, eyelid ellipse

This application visualises the 2D algorithm pipeline. It is easy to investigate certain blocks of the algorithm and improve them. The application can stream from video files or from cameras. For details on how to launch the application type ./starburst. The application uses a configuration file, described in Documents/settings/README. The application accepts a user-defined configuration file if provided. If not, it looks for ./default.xml and uses that if found, otherwise default values are used.

Usage: 
    ./starburst videofile [--track] [--config=filename.xml]
 Or 
    ./starburst cam=<number of device> [--track] [--config=filename.xml]

NOTE: Arto wrote most of this application. It might be a fantastic idea to rewrite some parts of the GUI stuff.



**********************************************************************
* /irist_finder/iris
**********************************************************************

This application was developed in the process of testing eyelid detection. The app opens up two windows. The first one shows the camera frame and the second one the detection results. Use this app in developing iris.cpp.

NOTE: place a "default.xml" eye tracking settings file in this folder if you want to define your own parameters.


**********************************************************************
TwoCameraTracker/gazetoworld/gazetoworld
**********************************************************************

This is the main gaze tracking application. The application opens up a window described below:

______________________
|   img1   |   img2   |
|__________|__________|
| buffers  | settings |
|__________|__________|

img1:    The eye camera output
img1:    The scene camera output
buffers: There are four buffers indicating (from top to bottom)

             1. Worker scene buffer
                 Queue for holding MJPG scene frames
             2. Worker eye buffer
                 Queue for holding MJPG eye frames
             3. Save buffer
                 Queue for storing eye frames, scene frames and track results
             4. Video buffer
                 Queue for storing decoded and analysed frames

settings: The slider settings, see Documents/settings/README for details.


The application processes either camera streams or it reads data from video files. The configuration is done with an xml file such as the the following:

<document>

	<settings id="output">
		<directory value="null" />
	</settings>

	<settings id="input_devices">
		<dev1 value="camera1.mjpg" />
		<dev2 value="camera2.mjpg" />
	</settings>

	<settings id="input_files">
		<gazetracker value = "config.xml" />
		<eyeCamCalibration value = "eyeCam.calib" />
		<sceneCamCalibration value = "sceneCam.calib" />
		<mapper value = "mapper.yaml" />
	</settings>

</document>


<directory value="some_directory" />:
This describes where the frames and the results are to be saved or if they need not be saved at all. There are three different alternatives:

    1. A valid directory, with the trailing '/' and <devX value="/dev/videoY">
       Creates a directory tree:
YYYYMMDDTHHMMSS (Timestamped, YYYY=year MM=month T=data-time-delimiter HH=hour MM=minute SS=second)
|
|---part0
|    |---camera1.mjpg
|    |---camera2.mjpg
|    |---results.res
|
|---part1
     |---camera1.mjpg
     |---camera2.mjpg
     |---results.res
...

|---partN
     |---camera1.mjpg
     |---camera2.mjpg
     |---results.res

      A new part will be created every 5 minutes.

    2. A valid directory, with the trailing '/' but <devX value="some_actual_video_file.mjpg">
       In this case only a results.res will be written into the folder

    3. "null"
       In this case nothing will be saved.


<devX value="camera_file.mjpg" /> or <devX value="/dev/videoX" />
    This defines the input. It can be either a camera or a .mjpg video file. Note that if dev1 is a camera then dev2 must be a camera as well. The same goes for video files.


<gazetracker value = "config.xml" />
    A configuration file, see Documents/settings/README

<eyeCamCalibration value = "eyeCam.calib" />
    A calibration file containing the camera data and the LED positions, there must be 6 LEDs, see Documents/LEDCalibration/README

<sceneCamCalibration value = "sceneCam.calib" />
    A calibration file containing the camera data, see Documents/LEDCalibration/README

<mapper value = "mapper.yaml" />
    A mapper matrix from the eye camera to the scene camera. Consult Huageng Chi for this. He has written an application to perform the rig calibration.


How to control the application:
Initially, the window displays both frames, the buffers and the settings. This process alone loads the processor, so in order to ease its load 'c'can be pressed which displays a rotating rectangle. This indicates that the measurement is running. This also prevents the DualFrameReceiver from feeding scene camera frames into the decoder, which is good.

Another secret button is the 'b' key. It will display the estimated cornea half-sphere with the LEDs rendered using a custom-made per-pixel shader.

The application starts recording once launched, unless <directory value="null" />. Therefore it is recommended that the user launches the application using "null", just to check that all is ok. The tracking parameter values can be adjusted by using the left and right arrow keys. Scrolling the parameters is done by up and down keys. The current parameter will be highlighted. Once the parameters are set write them down manually and adjust the configuration file <gazetracker value = "config.xml" />. If the tracking is bad, run starburst_searcher/starburst in order to try to identify the problems.

For further information on how video synchronisation works and when frames are dropped etc. read software_documentation/sync_videos/README.txt



**********************************************************************
TwoCameraTracker/resultvideo/result
**********************************************************************

The gazetoworld application saves camera frames and the reults into a binary file. In order to view and/or burn videos, i.e. display the results, this application may be used with various cmd parameters. For details on usage type ./result -help. This application takes a folder that must contain subfolders in the way described above in "TwoCameraTracker/gazetoworld/gazetoworld". Using that sample means that the -i option must be YYYYMMDDTHHMMSS.

