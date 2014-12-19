
# build type
ISDEBUG=true

# compiler
CC=g++

# flags
CFLAGS:=-c -g -Wall -pedantic -O0 `pkg-config --cflags --libs gstreamer-0.10` -lpthread

# libraries
LIBS:=-lSDL -lGL -ljpeg -lGLU -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_features2d -lopencv_calib3d -lm `sdl-config --libs` `gsl-config --libs` `pkg-config --cflags --libs gstreamer-0.10` -lgstvideo-0.10

# includes
INCLUDES:=	-I../GazeTracker/gazeTracker/		\
			-I../GazeTracker/pupil_tracker/		\
			-I../GazeTracker/cornea_tracker/	\
			-I../GazeTracker/scene_tracker/	\
			-I../GazeTracker/clusteriser/		\
			-I../starburst_searcher/		\
			-I../GazeTracker/settings_storage/	\
			-I../GazeTracker/					\
			-I../LedCalibration/				\
			-I../pattern_finder/				\
			-I../../Eigen2/						\
			-I../../tinyxml/					\
			-I../../VideoControl/					\
			-I../jpeg/						\
			`sdl-config --cflags`


# determine the build type
ifeq ($(ISDEBUG), true)
	INCLUDES+=-I/usr/local/src/OpenCV-2.3.1/build/debug/include/
	LIBS+=-L/usr/local/src/OpenCV-2.3.1/build/debug/lib
else
	INCLUDES+=-I/usr/local/src/OpenCV-2.3.1/build/release/include/
	LIBS+=-L/usr/local/src/OpenCV-2.3.1/build/release/lib
	CFLAGS+=-O2
endif



all: tracker


tracker: main.o PupilTracker.o starburst.o clusteriser.o Cornea_computer.o GazeTracker.o Camera.o settingsIO.o trackerSettings.o localTrackerSettings.o tinyxml.o tinystr.o tinyxmlerror.o tinyxmlparser.o CRTemplate.o SceneMapper.o group.o trackBar.o settingsPanel.o CaptureDevice.o jpeg.o VideoControl.o VideoHandler.o VideoBuffer.o CameraFrame.o JPEGWorker.o
	$(CC) main.o PupilTracker.o starburst.o clusteriser.o Cornea_computer.o GazeTracker.o Camera.o settingsIO.o trackerSettings.o localTrackerSettings.o tinyxml.o tinystr.o tinyxmlerror.o tinyxmlparser.o CRTemplate.o SceneMapper.o group.o trackBar.o settingsPanel.o CaptureDevice.o jpeg.o VideoControl.o VideoHandler.o VideoBuffer.o CameraFrame.o JPEGWorker.o -o tracker $(LIBS)


main.o: main.cpp
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp




CaptureDevice.o: ../../VideoControl/CaptureDevice.cpp ../../VideoControl/CaptureDevice.h ../../VideoControl/VideoControl.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../VideoControl/CaptureDevice.cpp


jpeg.o: ../jpeg/jpeg.cpp ../jpeg/jpeg.h
	$(CC) $(CFLAGS) $(INCLUDES) ../jpeg/jpeg.cpp


VideoControl.o: ../../VideoControl/VideoControl.cpp ../../VideoControl/VideoControl.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../VideoControl/VideoControl.cpp


VideoHandler.o: ../../VideoControl/VideoHandler.cpp ../../VideoControl/VideoHandler.h ../../VideoControl/VideoBuffer.h ../../VideoControl/CameraFrame.h ../../VideoControl/VideoControl.h ../../VideoControl/JPEGWorker.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../VideoControl/VideoHandler.cpp


VideoBuffer.o: ../../VideoControl/VideoBuffer.cpp ../../VideoControl/VideoBuffer.h ../../VideoControl/CaptureDevice.h ../../VideoControl/CameraFrame.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../VideoControl/VideoBuffer.cpp


CameraFrame.o: ../../VideoControl/CameraFrame.cpp ../../VideoControl/CameraFrame.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../VideoControl/CameraFrame.cpp


JPEGWorker.o: ../../VideoControl/JPEGWorker.cpp ../../VideoControl/JPEGWorker.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../VideoControl/JPEGWorker.cpp





GazeTracker.o: ../GazeTracker/gazeTracker/GazeTracker.h ../GazeTracker/gazeTracker/GazeTracker.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../GazeTracker/gazeTracker/GazeTracker.cpp


starburst.o: ../GazeTracker/pupil_tracker/starburst.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GazeTracker/pupil_tracker/starburst.cpp

settingsPanel.o: ../starburst_searcher/settingsPanel.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../starburst_searcher/settingsPanel.cpp

trackBar.o: ../starburst_searcher/trackBar.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../starburst_searcher/trackBar.cpp

PupilTracker.o: ../GazeTracker/pupil_tracker/PupilTracker.h ../GazeTracker/pupil_tracker/PupilTracker.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../GazeTracker/pupil_tracker/PupilTracker.cpp

	
clusteriser.o: ../GazeTracker/clusteriser/clusteriser.cpp ../GazeTracker/clusteriser/clusteriser.h
	$(CC) $(CFLAGS) $(INCLUDES) ../GazeTracker/clusteriser/clusteriser.cpp


Cornea_computer.o: ../GazeTracker/cornea_tracker/Cornea_computer.h ../GazeTracker/cornea_tracker/Cornea_computer.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../GazeTracker/cornea_tracker/Cornea_computer.cpp

SceneMapper.o: ../GazeTracker/scene_tracker/SceneMapper.h ../GazeTracker/scene_tracker/SceneMapper.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../GazeTracker/scene_tracker/SceneMapper.cpp

group.o: ../pattern_finder/group.h ../pattern_finder/group.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../pattern_finder/group.cpp

Camera.o: ../LedCalibration/Camera.h ../LedCalibration/Camera.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../LedCalibration/Camera.cpp

CRTemplate.o: ../GazeTracker/pupil_tracker/CRTemplate.h ../GazeTracker/pupil_tracker/CRTemplate.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GazeTracker/pupil_tracker/CRTemplate.cpp

settingsIO.o: ../GazeTracker/settings_storage/settingsIO.cpp ../GazeTracker/settings_storage/settingsIO.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GazeTracker/settings_storage/settingsIO.cpp

localTrackerSettings.o: ../GazeTracker/settings_storage/localTrackerSettings.cpp ../GazeTracker/settings_storage/localTrackerSettings.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GazeTracker/settings_storage/localTrackerSettings.cpp

trackerSettings.o: ../GazeTracker/settings_storage/trackerSettings.cpp ../GazeTracker/settings_storage/trackerSettings.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GazeTracker/settings_storage/trackerSettings.cpp

tinystr.o: ../../tinyxml/tinystr.cpp ../../tinyxml/tinystr.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../tinyxml/tinystr.cpp

tinyxml.o: ../../tinyxml/tinyxml.cpp ../../tinyxml/tinyxml.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../tinyxml/tinyxml.cpp

tinyxmlerror.o: ../../tinyxml/tinyxmlerror.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../tinyxml/tinyxmlerror.cpp


tinyxmlparser.o: ../../tinyxml/tinyxmlparser.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../tinyxml/tinyxmlparser.cpp


clean:
	rm -f *.o tracker

