
# build type
ISDEBUG=true

# compiler
CC=g++

# flags
CFLAGS:=-c -g -Wall -O0 `pkg-config --cflags --libs gstreamer-0.10` -lpthread

# libraries
LIBS:=-lSDL -lGL -lGLU -lopencv_core -lopencv_highgui -lopencv_calib3d -lopencv_imgproc -lm

# includes
INCLUDES:=	-I../../../LedCalibration/			\
			-I../../../../Eigen2/				\
			-I../../../../tinyxml/				\
			-I../../../../../MobileTracker/		\
			-I../../io/							\
			-I../../../../../Ganzheit/jpeg/



# determine the build type
ifeq ($(ISDEBUG), true)
	INCLUDES+=-I/usr/local/src/OpenCV-2.3.1/build/debug/include/
	LIBS+=-L/usr/local/src/OpenCV-2.3.1/build/debug/lib
else
	INCLUDES+=-I/usr/local/src/OpenCV-2.3.1/build/release/include/
	LIBS+=-L/usr/local/src/OpenCV-2.3.1/build/release/lib
	CFLAGS+=-O2
endif


PROG=xmldump


all: $(PROG)

$(PROG): main.o Camera.o tinyxml.o tinystr.o tinyxmlerror.o tinyxmlparser.o CalibDataReader.o
	$(CC) main.o Camera.o tinyxml.o tinystr.o tinyxmlerror.o tinyxmlparser.o CalibDataReader.o -o $(PROG) $(LIBS)


main.o: main.cpp ../../io/CalibDataReader.h
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp


Camera.o: ../../../LedCalibration/Camera.h ../../../LedCalibration/Camera.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../LedCalibration/Camera.cpp


tinystr.o: ../../../../tinyxml/tinystr.cpp ../../../../tinyxml/tinystr.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinystr.cpp

tinyxml.o: ../../../../tinyxml/tinyxml.cpp ../../../../tinyxml/tinyxml.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinyxml.cpp

tinyxmlerror.o: ../../../../tinyxml/tinyxmlerror.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinyxmlerror.cpp


tinyxmlparser.o: ../../../../tinyxml/tinyxmlparser.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../tinyxml/tinyxmlparser.cpp


CalibDataReader.o: ../../io/CalibDataReader.h ../../io/CalibDataReader.cpp
	$(CC) $(CFLAGS) $(INCLUDES) ../../io/CalibDataReader.cpp


clean:
	rm -f *.o $(PROG)

