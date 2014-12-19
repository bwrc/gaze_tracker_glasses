# build type
ISDEBUG=true

CC = g++

CFLAGS := -Wall -pedantic -Wextra -fopenmp

PROG = starburst

OPENCVDIR = /usr/local/src/OpenCV-2.4.0/
OPENTLD_DIR = /home/sharman/code/C++/gnebehay-OpenTLD/

INCLUDES:=
LIBS:=

INCLUDES:=	-I../../GazeTracker/settings_storage/	\
			-I../../GazeTracker/scene_tracker/		\
			-I../../GazeTracker/pupil_tracker/		\
			-I../../GazeTracker/cornea_tracker/		\
			-I../../GazeTracker/clusteriser/		\
			-I../../GazeTracker/					\
			-I../../pattern_finder/					\
			-I../../../Eigen/						\
			-I../../../tinyxml/

INCLUDES+= -I$(OPENTLD_DIR)src/


# determine the build type
ifeq ($(ISDEBUG), true)
	INCLUDES+= -I$(OPENCVDIR)build/debug/include/
	LIBS+= -L$(OPENCVDIR)build/debug/lib
	LIBS+= -L$(OPENTLD_DIR)debug/lib
	CFLAGS+=-g
else
	INCLUDES+=-I$(OPENCVDIR)build/release/include/
	LIBS+=-L$(OPENCVDIR)build/release/lib
	LIBS+= -L$(OPENTLD_DIR)release/lib/
	CFLAGS+=-O2
endif

# libraries
LIBS+= -lopencv_core -lopencv_imgproc -lopencv_highgui -lm -lopencv_features2d -lopencv_calib3d  -lopencv_objdetect -lopencv_video

# -lcvblobs must be the last library in the LIBS list
LIBS+= -lopentld -lgomp -lcvblobs


all: $(PROG)

$(PROG): main.o Timer.o starburst.o clusteriser.o PupilTracker.o SceneTracker.o svd.o settingsIO.o tinyxml.o tinystr.o tinyxmlerror.o tinyxmlparser.o CRTemplate.o trackerSettings.o settingsPanel.o trackBar.o localTrackerSettings.o group.o
	$(CC) -o $(PROG) main.o Timer.o starburst.o clusteriser.o PupilTracker.o SceneTracker.o svd.o settingsIO.o tinyxml.o tinystr.o tinyxmlerror.o tinyxmlparser.o CRTemplate.o trackerSettings.o settingsPanel.o trackBar.o localTrackerSettings.o group.o $(LIBS)


main.o: main.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp


settingsPanel.o: settingsPanel.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c settingsPanel.cpp


trackBar.o: trackBar.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c trackBar.cpp


Timer.o: Timer.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c Timer.cpp


PupilTracker.o: ../../GazeTracker/pupil_tracker/PupilTracker.h ../../GazeTracker/pupil_tracker/PupilTracker.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../GazeTracker/pupil_tracker/PupilTracker.cpp


CRTemplate.o: ../../GazeTracker/pupil_tracker/CRTemplate.h ../../GazeTracker/pupil_tracker/CRTemplate.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../GazeTracker/pupil_tracker/CRTemplate.cpp


clusteriser.o: ../../GazeTracker/clusteriser/clusteriser.cpp ../../GazeTracker/clusteriser/clusteriser.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../GazeTracker/clusteriser/clusteriser.cpp


starburst.o: ../../GazeTracker/pupil_tracker/starburst.cpp ../../GazeTracker/pupil_tracker/starburst.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../GazeTracker/pupil_tracker/starburst.cpp


SceneTracker.o: ../../GazeTracker/scene_tracker/SceneTracker.cpp ../../GazeTracker/scene_tracker/SceneTracker.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../GazeTracker/scene_tracker/SceneTracker.cpp


group.o: ../../pattern_finder/group.cpp ../../pattern_finder/group.h
	$(CC) $(CFLAGS) $(INCLUDES) -c  ../../pattern_finder/group.cpp


svd.o: ../../GazeTracker/scene_tracker/svd.c ../../GazeTracker/scene_tracker/svd.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../GazeTracker/scene_tracker/svd.c


settingsIO.o: ../../GazeTracker/settings_storage/settingsIO.cpp ../../GazeTracker/settings_storage/settingsIO.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../GazeTracker/settings_storage/settingsIO.cpp


trackerSettings.o: ../../GazeTracker/settings_storage/trackerSettings.cpp ../../GazeTracker/settings_storage/trackerSettings.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../GazeTracker/settings_storage/trackerSettings.cpp


localTrackerSettings.o: ../../GazeTracker/settings_storage/localTrackerSettings.cpp ../../GazeTracker/settings_storage/localTrackerSettings.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../GazeTracker/settings_storage/localTrackerSettings.cpp


tinystr.o: ../../../tinyxml/tinystr.cpp ../../../tinyxml/tinystr.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../tinyxml/tinystr.cpp


tinyxml.o: ../../../tinyxml/tinyxml.cpp ../../../tinyxml/tinyxml.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../tinyxml/tinyxml.cpp


tinyxmlerror.o: ../../../tinyxml/tinyxmlerror.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../tinyxml/tinyxmlerror.cpp


tinyxmlparser.o: ../../../tinyxml/tinyxmlparser.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../tinyxml/tinyxmlparser.cpp


clean:
	rm -f $(PROG) *.o

