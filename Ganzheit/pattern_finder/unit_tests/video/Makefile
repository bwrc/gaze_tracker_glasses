# build type
ISDEBUG=true

CC = g++

CFLAGS := -Wall -pedantic

PROG = video

OPENCVDIR = /usr/local/src/OpenCV-2.4.0/


INCLUDES:=	-I../../../GazeTracker/settings_storage/	\
			-I../../../GazeTracker/pupil_tracker/		\
			-I../../../GazeTracker/clusteriser/			\
			-I../../../GazeTracker/						\
			-I../../../../Eigen/						\
			-I../../									\
			-I../../../../tinyxml/


LIBS:=


# determine the build type
ifeq ($(ISDEBUG), true)
	INCLUDES+= -I$(OPENCVDIR)build/debug/include/
	LIBS+= -L$(OPENCVDIR)build/debug/lib
	CFLAGS+=-g
else
	INCLUDES+=-I$(OPENCVDIR)build/release/include/
	LIBS+=-L$(OPENCVDIR)build/release/lib
	CFLAGS+=-O2
endif

# libraries
LIBS+= -lopencv_core -lopencv_imgproc -lopencv_highgui -lm


all: $(PROG)




$(PROG): main.o group.o PupilTracker.o clusteriser.o settingsIO.o trackerSettings.o localTrackerSettings.o tinystr.o tinyxml.o  tinyxmlerror.o tinyxmlparser.o CRTemplate.o starburst.o
	$(CC) -o $(PROG) main.o group.o PupilTracker.o clusteriser.o settingsIO.o trackerSettings.o localTrackerSettings.o tinystr.o tinyxml.o tinyxmlerror.o tinyxmlparser.o CRTemplate.o starburst.o $(LIBS)


main.o: main.cpp ../../../GazeTracker/pupil_tracker/PupilTracker.h
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp


group.o: ../../group.cpp ../../group.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../group.cpp


PupilTracker.o: ../../../GazeTracker/pupil_tracker/PupilTracker.cpp ../../../GazeTracker/pupil_tracker/PupilTracker.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../GazeTracker/pupil_tracker/PupilTracker.cpp


CRTemplate.o: ../../../GazeTracker/pupil_tracker/CRTemplate.h ../../../GazeTracker/pupil_tracker/CRTemplate.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../GazeTracker/pupil_tracker/CRTemplate.cpp


clusteriser.o: ../../../GazeTracker/clusteriser/clusteriser.cpp ../../../GazeTracker/clusteriser/clusteriser.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../GazeTracker/clusteriser/clusteriser.cpp


settingsIO.o: ../../../GazeTracker/settings_storage/settingsIO.cpp ../../../GazeTracker/settings_storage/settingsIO.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../GazeTracker/settings_storage/settingsIO.cpp


trackerSettings.o: ../../../GazeTracker/settings_storage/trackerSettings.cpp ../../../GazeTracker/settings_storage/trackerSettings.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../GazeTracker/settings_storage/trackerSettings.cpp


starburst.o: ../../../GazeTracker/pupil_tracker/starburst.cpp ../../../GazeTracker/pupil_tracker/starburst.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../GazeTracker/pupil_tracker/starburst.cpp


localTrackerSettings.o: ../../../GazeTracker/settings_storage/localTrackerSettings.cpp ../../../GazeTracker/settings_storage/localTrackerSettings.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../GazeTracker/settings_storage/localTrackerSettings.cpp


tinystr.o: ../../../../tinyxml/tinystr.cpp ../../../../tinyxml/tinystr.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../../tinyxml/tinystr.cpp


tinyxml.o: ../../../../tinyxml/tinyxml.cpp ../../../../tinyxml/tinyxml.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../../tinyxml/tinyxml.cpp



tinyxmlerror.o: ../../../../tinyxml/tinyxmlerror.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../../tinyxml/tinyxmlerror.cpp


tinyxmlparser.o: ../../../../tinyxml/tinyxmlparser.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../../tinyxml/tinyxmlparser.cpp


clean:
	rm -f $(PROG) *.o

