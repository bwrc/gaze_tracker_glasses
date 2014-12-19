# build type
ISDEBUG=true

# compiler
CC=g++

# flags
CFLAGS:=-c -Wall -pedantic

# libraries
LIBS:=-lSDL -lGL -lopencv_core -lopencv_highgui -lopencv_calib3d -lopencv_imgproc -lopencv_features2d `sdl-config --libs` `gsl-config --libs`

# includes
INCLUDES:= -I../../                                    \
			`sdl-config --cflags` -I../../../../Eigen3 \
			-I../../../TwoCameraTracker/io/            \
			-I../../../../tinyxml

OPENCV_DIR = /usr/local/src/OpenCV-2.4.0/build/


# determine the build type
ifeq ($(ISDEBUG), true)
	INCLUDES+=-I$(OPENCV_DIR)/debug/include/
	LIBS+=-L$(OPENCV_DIR)debug/lib
	CFLAGS+=-g
else
	INCLUDES+=-I$(OPENCV_DIR)release/include/
	LIBS+=-L$(OPENCV_DIR)build/release/lib
	CFLAGS+=-O2
endif


PROG=samples


all: $(PROG)


$(PROG): main.o CalibPattern.o LED.o CameraWidget.o matrix.h Camera.o LEDCalibrator.o CameraCalibrator.o RectangleEstimator.o LEDCalibPattern.o imgproc.o LEDTracker.o tinyxmlparser.o tinyxmlerror.o tinyxml.o tinystr.o CalibDataWriter.o
	$(CC) main.o CalibPattern.o LED.o CameraWidget.o Camera.o LEDCalibrator.o CameraCalibrator.o RectangleEstimator.o LEDCalibPattern.o imgproc.o LEDTracker.o tinyxmlparser.o tinyxmlerror.o tinyxml.o tinystr.o CalibDataWriter.o -o $(PROG) $(LIBS)


main.o: main.cpp CalibPattern.h matrix.h LED.h GLDrawable.h CameraWidget.h ../../Camera.h
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp


CalibPattern.o: CalibPattern.cpp CalibPattern.h GLDrawable.h
	$(CC) $(CFLAGS) $(INCLUDES) CalibPattern.cpp


LED.o: LED.cpp LED.h GLDrawable.h
	$(CC) $(CFLAGS) $(INCLUDES) LED.cpp


CameraWidget.o: CameraWidget.cpp CameraWidget.h GLDrawable.h
	$(CC) $(CFLAGS) $(INCLUDES) CameraWidget.cpp


Camera.o: ../../Camera.cpp ../../Camera.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../Camera.cpp


LEDCalibrator.o: ../../LEDCalibrator.cpp ../../LEDCalibrator.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../LEDCalibrator.cpp

CameraCalibrator.o: ../../CameraCalibrator.cpp ../../CameraCalibrator.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../CameraCalibrator.cpp


RectangleEstimator.o: ../../RectangleEstimator.cpp ../../RectangleEstimator.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../RectangleEstimator.cpp


LEDCalibPattern.o: ../../LEDCalibPattern.cpp ../../LEDCalibPattern.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../LEDCalibPattern.cpp


LEDTracker.o: ../../LEDTracker.cpp ../../LEDTracker.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../LEDTracker.cpp

imgproc.o: ../../imgproc.cpp ../../imgproc.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../imgproc.cpp


tinystr.o: ../../../../tinyxml/tinystr.cpp ../../../../tinyxml/tinystr.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinystr.cpp


tinyxml.o: ../../../../tinyxml/tinyxml.cpp ../../../../tinyxml/tinyxml.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinyxml.cpp


tinyxmlerror.o: ../../../../tinyxml/tinyxmlerror.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinyxmlerror.cpp


tinyxmlparser.o: ../../../../tinyxml/tinyxmlparser.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinyxmlparser.cpp


CalibDataWriter.o: ../../../TwoCameraTracker/io/CalibDataWriter.cpp ../../../TwoCameraTracker/io/CalibDataWriter.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../TwoCameraTracker/io/CalibDataWriter.cpp

clean:
	rm -f *.o $(PROG)

