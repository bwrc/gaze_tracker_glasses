# build type
ISDEBUG=true

CC = g++

CFLAGS := -Wall -pedantic -c

PROG = calibrate

#OPENCVDIR = /usr/local/src/OpenCV-2.4.0/
OPENCVDIR = ../../../opencv/


INCLUDES:=	-I../io/                     \
			-I../../                     \
			-I../../LedCalibration/      \
			-I../../../tinyxml/          \
			-I../../../input_parser/     \
			-I../../TwoCameraTracker/io/ \
			-I../../../Eigen3/


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
LIBS+= -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_calib3d -lm 
LIBS+= -lopencv_features2d `gsl-config --libs`



all: $(PROG)


$(PROG): main.o Camera.o tinystr.o tinyxml.o tinyxmlerror.o tinyxmlparser.o InputParser.o CameraCalibrator.o LEDCalibrator.o LEDTracker.o imgproc.o CalibDataReader.o CalibDataWriter.o RectangleEstimator.o LEDCalibPattern.o
	$(CC) -o $(PROG) main.o Camera.o tinystr.o tinyxml.o tinyxmlerror.o tinyxmlparser.o InputParser.o CalibDataWriter.o CameraCalibrator.o LEDCalibrator.o LEDTracker.o imgproc.o CalibDataReader.o RectangleEstimator.o LEDCalibPattern.o $(LIBS)


main.o: main.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp


Camera.o: ../Camera.cpp ../Camera.h
	$(CC) $(CFLAGS) $(INCLUDES) ../Camera.cpp


CalibDataWriter.o:  ../../TwoCameraTracker/io/CalibDataWriter.cpp  ../../TwoCameraTracker/io/CalibDataWriter.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../TwoCameraTracker/io/CalibDataWriter.cpp


CameraCalibrator.o: ../CameraCalibrator.cpp ../CameraCalibrator.h
	$(CC) $(CFLAGS) $(INCLUDES) ../CameraCalibrator.cpp


LEDCalibrator.o: ../LEDCalibrator.cpp ../LEDCalibrator.h
	$(CC) $(CFLAGS) $(INCLUDES) ../LEDCalibrator.cpp


RectangleEstimator.o: ../RectangleEstimator.cpp ../RectangleEstimator.h
	$(CC) $(CFLAGS) $(INCLUDES) ../RectangleEstimator.cpp


LEDCalibPattern.o: ../LEDCalibPattern.cpp ../LEDCalibPattern.h
	$(CC) $(CFLAGS) $(INCLUDES) ../LEDCalibPattern.cpp


LEDTracker.o: ../LEDTracker.cpp ../LEDTracker.h
	$(CC) $(CFLAGS) $(INCLUDES) ../LEDTracker.cpp


imgproc.o: ../imgproc.cpp ../imgproc.h
	$(CC) $(CFLAGS) $(INCLUDES) ../imgproc.cpp


CalibDataReader.o: ../../TwoCameraTracker/io/CalibDataReader.cpp ../../TwoCameraTracker/io/CalibDataReader.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../TwoCameraTracker/io/CalibDataReader.cpp


tinystr.o: ../../../tinyxml/tinystr.cpp ../../../tinyxml/tinystr.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../tinyxml/tinystr.cpp


tinyxml.o: ../../../tinyxml/tinyxml.cpp ../../../tinyxml/tinyxml.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../tinyxml/tinyxml.cpp


tinyxmlerror.o: ../../../tinyxml/tinyxmlerror.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../tinyxml/tinyxmlerror.cpp


tinyxmlparser.o: ../../../tinyxml/tinyxmlparser.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../tinyxml/tinyxmlparser.cpp


InputParser.o: ../../../input_parser/InputParser.cpp ../../../input_parser/InputParser.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../../input_parser/InputParser.cpp


clean:
	rm -f $(PROG) *.o

