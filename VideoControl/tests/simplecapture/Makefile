
# build type
ISDEBUG=true

# compiler
CC=g++

# flags
CFLAGS:=-c -g -Wall -O0 `pkg-config --cflags --libs gstreamer-0.10` -lpthread

# libraries
LIBS:=-lSDL -ljpeg -lGL -lGLU -lopencv_core -lopencv_highgui -lopencv_calib3d -lopencv_imgproc -lm `sdl-config --libs` `pkg-config --cflags --libs gstreamer-0.10` -lgstvideo-0.10

# includes
INCLUDES:=	-I../../				\
			-I../../				\
			`sdl-config --cflags`

OPENCV_DIR=/usr/local/src/OpenCV-2.4.0/build/


# determine the build type
ifeq ($(ISDEBUG), true)
	INCLUDES+=-I$(OPENCV_DIR)debug/include/
	LIBS+=-L$(OPENCV_DIR)debug/lib
else
	INCLUDES+=-I$(OPENCV_DIR)release/include/
	LIBS+=-L$(OPENCV_DIR)release/lib
	CFLAGS+=-O2
endif


PROG = cam


all: $(PROG)


$(PROG): main.o CaptureDevice.o CameraFrame.o SimpleCapture.o
	$(CC) main.o CaptureDevice.o CameraFrame.o SimpleCapture.o -o $(PROG) $(LIBS)



main.o: main.cpp ../../CaptureDevice.h ../../SimpleCapture.h
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp


CaptureDevice.o: ../../CaptureDevice.cpp ../../CaptureDevice.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../CaptureDevice.cpp


CameraFrame.o: ../../CameraFrame.cpp ../../CameraFrame.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../CameraFrame.cpp


SimpleCapture.o: ../../SimpleCapture.cpp ../../SimpleCapture.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../SimpleCapture.cpp


clean:
	rm -f *.o $(PROG)

