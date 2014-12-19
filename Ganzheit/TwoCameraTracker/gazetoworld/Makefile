
# build type
ISDEBUG=true

# compiler
CC=g++

# flags
CFLAGS:=-c -Wall `pkg-config --cflags --libs gstreamer-0.10` -lpthread

# libraries
LIBS:= -Wl,-Bstatic -ltinyxml -Wl,-Bdynamic -lGL -lGLU -lGLEW -lopencv_core -lopencv_highgui -lopencv_calib3d -lopencv_imgproc -lm `sdl-config --libs` `gsl-config --libs` `pkg-config --cflags --libs gstreamer-0.10` -lgstvideo-0.10

# includes
INCLUDES:=	-Igui/									\
			-Iutils/								\
			-I../../GazeTracker/gazeTracker/		\
			-I../../GazeTracker/pupil_tracker/		\
			-I../../iris_finder/					\
			-I../../GazeTracker/ellipse/			\
			-I../../GazeTracker/cornea_tracker/		\
			-I../../GazeTracker/scene_tracker/		\
			-I../../GazeTracker/clusteriser/		\
			-I../../pattern_finder/					\
			-I../../GazeTracker/settings_storage/	\
			-I../../GazeTracker/					\
			-I../../ResultParser/					\
			-I../../LedCalibration/					\
			-I../../../VideoControl/				\
			-I../../../Eigen3/						\
			-I../../../tinyxml/						\
			-I../../../Ganzheit/jpeg/				\
			-I../../../thread/						\
			-I../io/								\
			-I../../../glsl							\
			`sdl-config --cflags`


OPENCV_DIR=../../../opencv/
TINYXML_DIR=../../../tinyxml/


# determine the build type
ifeq ($(ISDEBUG), true)
	INCLUDES+=-I$(OPENCV_DIR)build/debug/include/
	LIBS+=-L$(OPENCV_DIR)build/debug/lib
	LIBS+=-L$(TINYXML_DIR)build/debug
	CFLAGS+=-g
else
	INCLUDES+=-I$(OPENCV_DIR)build/release/include/
	LIBS+=-L$(OPENCV_DIR)build/release/lib
	LIBS+=-L$(TINYXML_DIR)build/release
	CFLAGS+=-O2
endif


PROG=gazetoworld


OBJECTS = main.o PupilTracker.o iris.o ellipse.o starburst.o clusteriser.o Cornea_computer.o GazeTracker.o Camera.o settingsIO.o trackerSettings.o localTrackerSettings.o CRTemplate.o SceneMapper.o group.o GLVideoCanvas.o DualFrameReceiver.o CameraFrame.o StreamWorker.o JPEGWorker.o GTWorker.o jpeg.o CaptureDevice.o VideoControl.o Settings.o GLWidget.o BufferWidget.o VideoWriter.o SettingsPanel.o CalibDataReader.o ResultData.o BinaryResultParser.o PanelIdle.o MapperReader.o Thread.o VideoSync.o SimpleCapture.o ResultWriter.o GLCornea.o Shader.o


all: $(PROG)


$(PROG): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROG) $(LIBS)


main.o: main.cpp
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp


group.o: ../../pattern_finder/group.h ../../pattern_finder/group.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../pattern_finder/group.cpp


GazeTracker.o: ../../GazeTracker/gazeTracker/GazeTracker.h ../../GazeTracker/gazeTracker/GazeTracker.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../GazeTracker/gazeTracker/GazeTracker.cpp


starburst.o: ../../GazeTracker/pupil_tracker/starburst.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../GazeTracker/pupil_tracker/starburst.cpp


PupilTracker.o: ../../GazeTracker/pupil_tracker/PupilTracker.h ../../GazeTracker/pupil_tracker/PupilTracker.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../GazeTracker/pupil_tracker/PupilTracker.cpp


iris.o: ../../iris_finder/iris.cpp ../../iris_finder/iris.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../iris_finder/iris.cpp


ellipse.o: ../../GazeTracker/ellipse/ellipse.cpp ../../GazeTracker/ellipse/ellipse.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../GazeTracker/ellipse/ellipse.cpp


clusteriser.o: ../../GazeTracker/clusteriser/clusteriser.cpp ../../GazeTracker/clusteriser/clusteriser.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../GazeTracker/clusteriser/clusteriser.cpp


CRTemplate.o: ../../GazeTracker/pupil_tracker/CRTemplate.h ../../GazeTracker/pupil_tracker/CRTemplate.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../GazeTracker/pupil_tracker/CRTemplate.cpp


Cornea_computer.o: ../../GazeTracker/cornea_tracker/Cornea_computer.h ../../GazeTracker/cornea_tracker/Cornea_computer.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../GazeTracker/cornea_tracker/Cornea_computer.cpp


SceneMapper.o: ../../GazeTracker/scene_tracker/SceneMapper.h ../../GazeTracker/scene_tracker/SceneMapper.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../GazeTracker/scene_tracker/SceneMapper.cpp


Camera.o: ../../LedCalibration/Camera.h ../../LedCalibration/Camera.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../LedCalibration/Camera.cpp


settingsIO.o: ../../GazeTracker/settings_storage/settingsIO.cpp ../../GazeTracker/settings_storage/settingsIO.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../GazeTracker/settings_storage/settingsIO.cpp


localTrackerSettings.o: ../../GazeTracker/settings_storage/localTrackerSettings.cpp ../../GazeTracker/settings_storage/localTrackerSettings.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../GazeTracker/settings_storage/localTrackerSettings.cpp


trackerSettings.o: ../../GazeTracker/settings_storage/trackerSettings.cpp ../../GazeTracker/settings_storage/trackerSettings.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../GazeTracker/settings_storage/trackerSettings.cpp


VideoSync.o: VideoSync.cpp VideoSync.h
	$(CC) $(CFLAGS) $(INCLUDES) VideoSync.cpp


CameraFrame.o: ../../../VideoControl/CameraFrame.cpp ../../../VideoControl/CameraFrame.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../VideoControl/CameraFrame.cpp


DualFrameReceiver.o: DualFrameReceiver.cpp DualFrameReceiver.h
	$(CC) $(CFLAGS) $(INCLUDES) DualFrameReceiver.cpp


JPEGWorker.o: ../../../VideoControl/JPEGWorker.cpp ../../../VideoControl/JPEGWorker.h ../../../VideoControl/StreamWorker.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../VideoControl/JPEGWorker.cpp


GTWorker.o: GTWorker.cpp GTWorker.h ../../../VideoControl/StreamWorker.h
	$(CC) $(CFLAGS) $(INCLUDES) GTWorker.cpp


StreamWorker.o: ../../../VideoControl/StreamWorker.cpp ../../../VideoControl/StreamWorker.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../VideoControl/StreamWorker.cpp


jpeg.o: ../../../Ganzheit/jpeg/jpeg.cpp ../../../Ganzheit/jpeg/jpeg.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../Ganzheit/jpeg/jpeg.cpp


CaptureDevice.o: ../../../VideoControl/CaptureDevice.cpp ../../../VideoControl/CaptureDevice.h ../../../VideoControl/VideoControl.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../VideoControl/CaptureDevice.cpp


SimpleCapture.o: ../../../VideoControl/SimpleCapture.cpp ../../../VideoControl/SimpleCapture.h ../../../VideoControl/VideoControl.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../VideoControl/SimpleCapture.cpp


VideoControl.o: ../../../VideoControl/VideoControl.cpp ../../../VideoControl/VideoControl.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../VideoControl/VideoControl.cpp


GLVideoCanvas.o: gui/GLVideoCanvas.cpp gui/GLVideoCanvas.h
	$(CC) $(CFLAGS) $(INCLUDES) gui/GLVideoCanvas.cpp


GLWidget.o: gui/GLWidget.cpp gui/GLWidget.h
	$(CC) $(CFLAGS) $(INCLUDES) gui/GLWidget.cpp


PanelIdle.o: gui/PanelIdle.cpp gui/PanelIdle.h
	$(CC) $(CFLAGS) $(INCLUDES) gui/PanelIdle.cpp


SettingsPanel.o: gui/SettingsPanel.cpp gui/SettingsPanel.h
	$(CC) $(CFLAGS) $(INCLUDES) gui/SettingsPanel.cpp


BufferWidget.o: gui/BufferWidget.cpp gui/BufferWidget.h
	$(CC) $(CFLAGS) $(INCLUDES) gui/BufferWidget.cpp


Settings.o: ../io/Settings.cpp ../io/Settings.h
	$(CC) $(CFLAGS) $(INCLUDES) ../io/Settings.cpp


MapperReader.o: ../io/MapperReader.cpp ../io/MapperReader.h
	$(CC) $(CFLAGS) $(INCLUDES) ../io/MapperReader.cpp


VideoWriter.o: VideoWriter.cpp VideoWriter.h DataWriter.h
	$(CC) $(CFLAGS) $(INCLUDES) VideoWriter.cpp


ResultWriter.o: ResultWriter.cpp VideoWriter.h ResultWriter.h
	$(CC) $(CFLAGS) $(INCLUDES) ResultWriter.cpp


CalibDataReader.o: ../io/CalibDataReader.cpp ../io/CalibDataReader.h
	$(CC) $(CFLAGS) $(INCLUDES) ../io/CalibDataReader.cpp


ResultData.o: ../../ResultParser/ResultData.cpp ../../ResultParser/ResultData.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../ResultParser/ResultData.cpp


BinaryResultParser.o: ../../ResultParser/BinaryResultParser.cpp ../../ResultParser/BinaryResultParser.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../ResultParser/BinaryResultParser.cpp


Thread.o:  ../../../thread/Thread.cpp  ../../../thread/Thread.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../thread/Thread.cpp


GLCornea.o: gui/GLCornea.cpp gui/GLCornea.h
	$(CC) $(CFLAGS) $(INCLUDES) gui/GLCornea.cpp


Shader.o: ../../../glsl/Shader.cpp ../../../glsl/Shader.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../glsl/Shader.cpp


clean:
	rm -f *.o $(PROG)

