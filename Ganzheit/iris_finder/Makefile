
# build type
ISDEBUG=true

# compiler
CC=g++

# flags
CFLAGS:=-c -Wall -lpthread

# libraries

# includes
INCLUDES:=	-I.											\
			-I../../input_parser/						\
			-I../TwoCameraTracker/io/					\
			-I../GazeTracker/pupil_tracker/   	 		\
			-I../GazeTracker/ellipse					\
			-I../TwoCameraTracker/gazetoworld/utils/	\
			-I../GazeTracker/scene_tracker/				\
			-I../GazeTracker/clusteriser/				\
			-I../pattern_finder/						\
			-I../GazeTracker/settings_storage/			\
			-I../GazeTracker/							\
			-I../ResultParser/							\
			-I../LedCalibration/						\
			-I../../Eigen3/								\
			-I../../tinyxml/							\
			-I../../.thread/


OPENCV_DIR=../../opencv/
TINY_XML_DIR=../../tinyxml/build/

LIBS+=-L.

# determine the build type
ifeq ($(ISDEBUG), true)
	INCLUDES+=-I$(OPENCV_DIR)build/debug/include/
	LIBS+=-L$(OPENCV_DIR)build/debug/lib
	LIBS+=-L$(TINY_XML_DIR)debug
	CFLAGS+=-g
else
	INCLUDES+=-I$(OPENCV_DIR)build/release/include/
	LIBS+=-L$(OPENCV_DIR)build/release/lib
	LIBS+=-L$(TINY_XML_DIR)release
	CFLAGS+=-O2
endif

LIBS+=-Wl,-Bstatic -ltinyxml -Wl,-Bdynamic -lopencv_core -lopencv_highgui -lopencv_calib3d -lopencv_imgproc -lm



PROG=iris

OBJECTS = main.o PupilTracker.o starburst.o clusteriser.o settingsIO.o trackerSettings.o localTrackerSettings.o CRTemplate.o ResultData.o Thread.o InputParser.o iris.o ellipse.o


all: $(PROG)


$(PROG): $(OBJECTS)
	$(CC) -o $(PROG) $(OBJECTS) $(LIBS)


main.o: main.cpp Averager.h iris.h
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp


starburst.o: ../GazeTracker/pupil_tracker/starburst.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../GazeTracker/pupil_tracker/starburst.cpp


PupilTracker.o: ../GazeTracker/pupil_tracker/PupilTracker.h ../GazeTracker/pupil_tracker/PupilTracker.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../GazeTracker/pupil_tracker/PupilTracker.cpp


iris.o: iris.cpp iris.h
	$(CC) $(CFLAGS) $(INCLUDES) iris.cpp


ellipse.o: ../GazeTracker/ellipse/ellipse.cpp ../GazeTracker/ellipse/ellipse.h
	$(CC) $(CFLAGS) $(INCLUDES) ../GazeTracker/ellipse/ellipse.cpp


clusteriser.o: ../GazeTracker/clusteriser/clusteriser.cpp ../GazeTracker/clusteriser/clusteriser.h
	$(CC) $(CFLAGS) $(INCLUDES) ../GazeTracker/clusteriser/clusteriser.cpp


CRTemplate.o: ../GazeTracker/pupil_tracker/CRTemplate.h ../GazeTracker/pupil_tracker/CRTemplate.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../GazeTracker/pupil_tracker/CRTemplate.cpp


settingsIO.o: ../GazeTracker/settings_storage/settingsIO.cpp ../GazeTracker/settings_storage/settingsIO.h
	$(CC) $(CFLAGS) $(INCLUDES) ../GazeTracker/settings_storage/settingsIO.cpp


localTrackerSettings.o: ../GazeTracker/settings_storage/localTrackerSettings.cpp ../GazeTracker/settings_storage/localTrackerSettings.h
	$(CC) $(CFLAGS) $(INCLUDES) ../GazeTracker/settings_storage/localTrackerSettings.cpp


trackerSettings.o: ../GazeTracker/settings_storage/trackerSettings.cpp ../GazeTracker/settings_storage/trackerSettings.h
	$(CC) $(CFLAGS) $(INCLUDES) ../GazeTracker/settings_storage/trackerSettings.cpp


ResultData.o: ../ResultParser/ResultData.cpp ../ResultParser/ResultData.h
	$(CC) $(CFLAGS) $(INCLUDES) ../ResultParser/ResultData.cpp


Thread.o:  ../../thread/Thread.cpp  ../../thread/Thread.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../thread/Thread.cpp


InputParser.o: ../../input_parser/InputParser.cpp ../../input_parser/InputParser.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../input_parser/InputParser.cpp


clean:
	rm -f *.o $(PROG)

