# build type
ISDEBUG=true

CC = g++

CFLAGS := -Wall -pedantic

PROG = result

#OPENCVDIR = /usr/local/src/OpenCV-2.4.0/
OPENCVDIR = ../../../opencv/


INCLUDES:=	-I../../ResultParser/     \
			-I../io/                  \
			-I../../LedCalibration/   \
			-I../../../tinyxml/       \
			-I../../../input_parser/  \
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


all: $(PROG)


$(PROG): main.o BinaryResultParser.o Camera.o tinystr.o tinyxml.o tinyxmlerror.o tinyxmlparser.o ResultData.o ResultStreamer.o InputParser.o
	$(CC) -o $(PROG) main.o BinaryResultParser.o Camera.o tinystr.o tinyxml.o tinyxmlerror.o tinyxmlparser.o ResultData.o ResultStreamer.o InputParser.o $(LIBS)


main.o: main.cpp ../../ResultParser/ResultParser.h
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp


ResultData.o: ../../ResultParser/ResultData.cpp ../../ResultParser/ResultData.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../ResultParser/ResultData.cpp


BinaryResultParser.o: ../../ResultParser/BinaryResultParser.cpp ../../ResultParser/BinaryResultParser.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../ResultParser/BinaryResultParser.cpp


Camera.o: ../../LedCalibration/Camera.cpp ../../LedCalibration/Camera.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../LedCalibration/Camera.cpp


ResultStreamer.o: ResultStreamer.cpp ResultStreamer.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ResultStreamer.cpp


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

