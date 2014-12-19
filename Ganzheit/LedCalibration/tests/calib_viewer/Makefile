# build type
ISDEBUG=true

# compiler
CC=g++

# flags
CFLAGS:=-c -Wall -pedantic

# libraries
LIBS:=-lSDL -lGL -lopencv_core -lopencv_highgui -lopencv_calib3d -lopencv_imgproc -lopencv_features2d `sdl-config --libs` `gsl-config --libs`

# includes
INCLUDES:= -I../../  						 \
		   `sdl-config --cflags` 			 \
			-I../../../../Eigen3             \
			-I../../../../tinyxml			 \
			-I../../../../input_parser		 \
			-I../../../TwoCameraTracker/io/

#OPENCV_DIR = /usr/local/src/OpenCV-2.4.0/build/
OPENCV_DIR = ../../../../opencv/build/


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


$(PROG): main.o CalibPattern.o  matrix.o Camera.o LEDCalibrator.o RectangleEstimator.o LEDCalibPattern.o imgproc.o LEDTracker.o CalibDataReader.o tinyxmlparser.o tinyxmlerror.o tinyxml.o tinystr.o matrix.o GrSamples.o CameraCalibrator.o InputParser.o CalibDataWriter.o
	$(CC) main.o CalibPattern.o Camera.o LEDCalibrator.o RectangleEstimator.o LEDCalibPattern.o imgproc.o LEDTracker.o CalibDataReader.o tinyxmlparser.o tinyxmlerror.o tinyxml.o tinystr.o matrix.o GrSamples.o CameraCalibrator.o InputParser.o CalibDataWriter.o -o $(PROG) $(LIBS)


main.o: main.cpp CalibPattern.h matrix.h GLDrawable.h ../../Camera.h
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp


CalibPattern.o: CalibPattern.cpp CalibPattern.h GLDrawable.h
	$(CC) $(CFLAGS) $(INCLUDES) CalibPattern.cpp


GrSamples.o: GrSamples.cpp GrSamples.h
	$(CC) $(CFLAGS) $(INCLUDES) GrSamples.cpp


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


CalibDataReader.o: ../../../TwoCameraTracker/io/CalibDataReader.cpp ../../../TwoCameraTracker/io/CalibDataReader.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../TwoCameraTracker/io/CalibDataReader.cpp

matrix.o: matrix.cpp matrix.h
	$(CC) $(CFLAGS) $(INCLUDES) matrix.cpp


tinystr.o: ../../../../tinyxml/tinystr.cpp ../../../../tinyxml/tinystr.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinystr.cpp


tinyxml.o: ../../../../tinyxml/tinyxml.cpp ../../../../tinyxml/tinyxml.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinyxml.cpp


tinyxmlerror.o: ../../../../tinyxml/tinyxmlerror.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinyxmlerror.cpp


tinyxmlparser.o: ../../../../tinyxml/tinyxmlparser.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinyxmlparser.cpp


InputParser.o: ../../../../input_parser/InputParser.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../input_parser/InputParser.cpp


CalibDataWriter.o: ../../../TwoCameraTracker/io/CalibDataWriter.cpp  ../../../TwoCameraTracker/io/CalibDataWriter.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../TwoCameraTracker/io/CalibDataWriter.cpp


clean:
	rm -f *.o $(PROG)

