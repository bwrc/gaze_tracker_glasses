# build type
ISDEBUG=true

CC = g++

CFLAGS := -Wall -pedantic

PROG = starburst

OPENCVDIR = ../../opencv/


INCLUDES:=
LIBS:=

INCLUDES:=	-I../GazeTracker/settings_storage/	\
			-I../GazeTracker/scene_tracker/		\
			-I../GazeTracker/pupil_tracker/		\
			-I../GazeTracker/ellipse/			\
			-I../iris_finder/					\
			-I../GazeTracker/cornea_tracker/	\
			-I../GazeTracker/clusteriser/		\
			-I../TwoCameraTracker/gazetoworld/utils/		\
			-I../GazeTracker/					\
			-I../../Eigen/						\
			-I../../tinyxml/



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

OBJECTS = main.o Timer.o starburst.o clusteriser.o PupilTracker.o iris.o ellipse.o SceneTracker.o settingsIO.o tinyxml.o tinystr.o tinyxmlerror.o tinyxmlparser.o CRTemplate.o trackerSettings.o settingsPanel.o trackBar.o localTrackerSettings.o svd.o


all: $(PROG)

$(PROG): $(OBJECTS)
	$(CC) -o $(PROG) $(OBJECTS) $(LIBS)


main.o: main.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp


settingsPanel.o: settingsPanel.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c settingsPanel.cpp


trackBar.o: trackBar.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c trackBar.cpp


Timer.o: Timer.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c Timer.cpp


PupilTracker.o: ../GazeTracker/pupil_tracker/PupilTracker.h ../GazeTracker/pupil_tracker/PupilTracker.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GazeTracker/pupil_tracker/PupilTracker.cpp


ellipse.o: ../GazeTracker/ellipse/ellipse.cpp ../GazeTracker/ellipse/ellipse.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GazeTracker/ellipse/ellipse.cpp


iris.o: ../iris_finder/iris.cpp ../iris_finder/iris.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../iris_finder/iris.cpp


CRTemplate.o: ../GazeTracker/pupil_tracker/CRTemplate.h ../GazeTracker/pupil_tracker/CRTemplate.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GazeTracker/pupil_tracker/CRTemplate.cpp


clusteriser.o: ../GazeTracker/clusteriser/clusteriser.cpp ../GazeTracker/clusteriser/clusteriser.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GazeTracker/clusteriser/clusteriser.cpp


starburst.o: ../GazeTracker/pupil_tracker/starburst.cpp ../GazeTracker/pupil_tracker/starburst.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GazeTracker/pupil_tracker/starburst.cpp


SceneTracker.o: ../GazeTracker/scene_tracker/SceneTracker.cpp ../GazeTracker/scene_tracker/SceneTracker.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GazeTracker/scene_tracker/SceneTracker.cpp

svd.o: ../GazeTracker/scene_tracker/svd.c ../GazeTracker/scene_tracker/svd.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GazeTracker/scene_tracker/svd.c



settingsIO.o: ../GazeTracker/settings_storage/settingsIO.cpp ../GazeTracker/settings_storage/settingsIO.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GazeTracker/settings_storage/settingsIO.cpp

trackerSettings.o: ../GazeTracker/settings_storage/trackerSettings.cpp ../GazeTracker/settings_storage/trackerSettings.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GazeTracker/settings_storage/trackerSettings.cpp

localTrackerSettings.o: ../GazeTracker/settings_storage/localTrackerSettings.cpp ../GazeTracker/settings_storage/localTrackerSettings.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../GazeTracker/settings_storage/localTrackerSettings.cpp

tinystr.o: ../../tinyxml/tinystr.cpp ../../tinyxml/tinystr.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../tinyxml/tinystr.cpp

tinyxml.o: ../../tinyxml/tinyxml.cpp ../../tinyxml/tinyxml.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../tinyxml/tinyxml.cpp

tinyxmlerror.o: ../../tinyxml/tinyxmlerror.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../tinyxml/tinyxmlerror.cpp


tinyxmlparser.o: ../../tinyxml/tinyxmlparser.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../tinyxml/tinyxmlparser.cpp



clean:
	rm -f $(PROG) *.o

