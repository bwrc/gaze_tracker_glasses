
# build type
ISDEBUG=true

# compiler
CC=g++

# flags
CFLAGS:=-c -g -Wall -O0 `pkg-config --cflags --libs gstreamer-0.10` -lpthread

# libraries
LIBS:=-lopencv_core -lopencv_highgui -lopencv_calib3d -lopencv_imgproc -lm `gsl-config --libs` `pkg-config --cflags --libs gstreamer-0.10` -lgstvideo-0.10

# includes
INCLUDES:=	-Igui/										\
			-I../../../../thread						\
			-I../../../GazeTracker/gazeTracker/			\
			-I../../../GazeTracker/pupil_tracker/		\
			-I../../../GazeTracker/cornea_tracker/		\
			-I../../../GazeTracker/scene_tracker/		\
			-I../../../GazeTracker/clusteriser/			\
			-I../../../pattern_finder/					\
			-I../../../iris_finder/						\
			-I../../../GazeTracker/ellipse				\
			-I../../gazetoworld/utils					\
			-I../../../GazeTracker/settings_storage/	\
			-I../../../GazeTracker/						\
			-I../../../LedCalibration/					\
			-I../../../../VideoControl/					\
			-I../../../../Eigen3/						\
			-I../../../../tinyxml/						\
			-I../../../../MobileTracker/				\
			-I../../../../Ganzheit/jpeg/				\
			-I../../io/									\
			-I../../gazetoworld/						\
			-I../../../ResultParser/					\
			-I../../socket_communication/


# determine the build type
ifeq ($(ISDEBUG), true)
	INCLUDES+=-I/usr/local/src/OpenCV-2.4.0/build/debug/include/
	LIBS+=-L/usr/local/src/OpenCV-2.4.0/build/debug/lib
else
	INCLUDES+=-I/usr/local/src/OpenCV-2.4.0/build/release/include/
	LIBS+=-L/usr/local/src/OpenCV-2.4.0/build/release/lib
	CFLAGS+=-O2
endif


BIN=bin
PROG=client

OBJECTS=main.o PupilTracker.o starburst.o clusteriser.o Cornea_computer.o GazeTracker.o Camera.o settingsIO.o trackerSettings.o localTrackerSettings.o tinyxml.o tinystr.o tinyxmlerror.o tinyxmlparser.o CRTemplate.o SceneMapper.o group.o VideoHandler.o DualFrameReceiver.o CameraFrame.o StreamWorker.o JPEGWorker.o GTWorker.o jpeg.o CaptureDevice.o VideoControl.o Settings.o DataSink.o CalibDataReader.o Communicator.o Client.o ResultData.o BinaryResultParser.o MapperReader.o iris.o ellipse.o Thread.o

all: $(PROG)

$(PROG): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROG) $(LIBS)


main.o: main.cpp
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp


Thread.o: ../../../../thread/Thread.cpp ../../../../thread/Thread.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../thread/Thread.cpp

group.o: ../../../pattern_finder/group.h ../../../pattern_finder/group.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../pattern_finder/group.cpp


GazeTracker.o: ../../../GazeTracker/gazeTracker/GazeTracker.h ../../../GazeTracker/gazeTracker/GazeTracker.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../GazeTracker/gazeTracker/GazeTracker.cpp


starburst.o: ../../../GazeTracker/pupil_tracker/starburst.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../GazeTracker/pupil_tracker/starburst.cpp


PupilTracker.o: ../../../GazeTracker/pupil_tracker/PupilTracker.h ../../../GazeTracker/pupil_tracker/PupilTracker.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../GazeTracker/pupil_tracker/PupilTracker.cpp

iris.o: ../../../iris_finder/iris.cpp ../../../iris_finder/iris.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../iris_finder/iris.cpp


ellipse.o: ../../../GazeTracker/ellipse/ellipse.cpp ../../../GazeTracker/ellipse/ellipse.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../GazeTracker/ellipse/ellipse.cpp


clusteriser.o: ../../../GazeTracker/clusteriser/clusteriser.cpp ../../../GazeTracker/clusteriser/clusteriser.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../GazeTracker/clusteriser/clusteriser.cpp


Cornea_computer.o: ../../../GazeTracker/cornea_tracker/Cornea_computer.h ../../../GazeTracker/cornea_tracker/Cornea_computer.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../GazeTracker/cornea_tracker/Cornea_computer.cpp

SceneMapper.o: ../../../GazeTracker/scene_tracker/SceneMapper.h ../../../GazeTracker/scene_tracker/SceneMapper.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../GazeTracker/scene_tracker/SceneMapper.cpp

Camera.o: ../../../LedCalibration/Camera.h ../../../LedCalibration/Camera.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../LedCalibration/Camera.cpp

CRTemplate.o: ../../../GazeTracker/pupil_tracker/CRTemplate.h ../../../GazeTracker/pupil_tracker/CRTemplate.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../GazeTracker/pupil_tracker/CRTemplate.cpp

settingsIO.o: ../../../GazeTracker/settings_storage/settingsIO.cpp ../../../GazeTracker/settings_storage/settingsIO.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../GazeTracker/settings_storage/settingsIO.cpp

localTrackerSettings.o: ../../../GazeTracker/settings_storage/localTrackerSettings.cpp ../../../GazeTracker/settings_storage/localTrackerSettings.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../GazeTracker/settings_storage/localTrackerSettings.cpp

trackerSettings.o: ../../../GazeTracker/settings_storage/trackerSettings.cpp ../../../GazeTracker/settings_storage/trackerSettings.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../GazeTracker/settings_storage/trackerSettings.cpp

tinystr.o: ../../../../tinyxml/tinystr.cpp ../../../../tinyxml/tinystr.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinystr.cpp

tinyxml.o: ../../../../tinyxml/tinyxml.cpp ../../../../tinyxml/tinyxml.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinyxml.cpp

tinyxmlerror.o: ../../../../tinyxml/tinyxmlerror.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinyxmlerror.cpp

tinyxmlparser.o: ../../../../tinyxml/tinyxmlparser.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinyxmlparser.cpp


VideoHandler.o: ../../../../VideoControl/VideoHandler.cpp ../../../../VideoControl/VideoHandler.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../VideoControl/VideoHandler.cpp


CameraFrame.o: ../../../../VideoControl/CameraFrame.cpp ../../../../VideoControl/CameraFrame.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../VideoControl/CameraFrame.cpp


DualFrameReceiver.o: DualFrameReceiver.cpp DualFrameReceiver.h
	$(CC) $(CFLAGS) $(INCLUDES) DualFrameReceiver.cpp


JPEGWorker.o: ../../../../VideoControl/JPEGWorker.cpp ../../../../VideoControl/JPEGWorker.h ../../../../VideoControl/StreamWorker.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../VideoControl/JPEGWorker.cpp


GTWorker.o: ../../gazetoworld/GTWorker.cpp ../../gazetoworld/GTWorker.h ../../../../VideoControl/StreamWorker.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../gazetoworld/GTWorker.cpp


StreamWorker.o: ../../../../VideoControl/StreamWorker.cpp ../../../../VideoControl/StreamWorker.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../VideoControl/StreamWorker.cpp

jpeg.o: ../../../../Ganzheit/jpeg/jpeg.cpp ../../../../Ganzheit/jpeg/jpeg.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../Ganzheit/jpeg/jpeg.cpp


CaptureDevice.o: ../../../../VideoControl/CaptureDevice.cpp ../../../../VideoControl/CaptureDevice.h ../../../../VideoControl/VideoControl.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../VideoControl/CaptureDevice.cpp

VideoControl.o: ../../../../VideoControl/VideoControl.cpp ../../../../VideoControl/VideoControl.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../VideoControl/VideoControl.cpp


Settings.o: ../../io/Settings.cpp ../../io/Settings.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../io/Settings.cpp


MapperReader.o: ../../io/MapperReader.cpp ../../io/MapperReader.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../io/MapperReader.cpp


DataSink.o: DataSink.cpp DataSink.h
	$(CC) $(CFLAGS) $(INCLUDES) DataSink.cpp

CalibDataReader.o: ../../io/CalibDataReader.cpp ../../io/CalibDataReader.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../io/CalibDataReader.cpp


Communicator.o: ../../socket_communication/Communicator.cpp ../../socket_communication/Communicator.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../socket_communication/Communicator.cpp


Client.o: ../../socket_communication/Client.cpp ../../socket_communication/Client.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../socket_communication/Client.cpp


ResultData.o: ../../../ResultParser/ResultData.h ../../../ResultParser/ResultData.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../ResultParser/ResultData.cpp


BinaryResultParser.o: ../../../ResultParser/BinaryResultParser.h ../../../ResultParser/BinaryResultParser.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../ResultParser/BinaryResultParser.cpp





clean:
	rm -f *.o $(PROG)

