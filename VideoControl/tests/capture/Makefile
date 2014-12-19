
# build type
ISDEBUG=true

# compiler
CC=g++

# flags
CFLAGS:=-c -g -Wall -O0 `pkg-config --cflags --libs gstreamer-0.10` -lpthread

# libraries
LIBS:=-lSDL -ljpeg -lGL -lGLU -lopencv_core -lopencv_highgui -lopencv_calib3d -lopencv_imgproc -lm `sdl-config --libs` `pkg-config --cflags --libs gstreamer-0.10` -lgstvideo-0.10

# includes
INCLUDES:=	-I../../../Ganzheit/jpeg/		\
			-I../../						\
			-I../../						\
			`sdl-config --cflags`


# determine the build type
ifeq ($(ISDEBUG), true)
	INCLUDES+=-I/usr/local/src/OpenCV-2.3.1/build/debug/include/
	LIBS+=-L/usr/local/src/OpenCV-2.3.1/build/debug/lib
else
	INCLUDES+=-I/usr/local/src/OpenCV-2.3.1/build/release/include/
	LIBS+=-L/usr/local/src/OpenCV-2.3.1/build/release/lib
	CFLAGS+=-O2
endif




PROG = cam


all: $(PROG)


$(PROG): main.o CaptureDevice.o jpeg.o VideoControl.o CameraFrame.o
	$(CC) main.o CaptureDevice.o jpeg.o VideoControl.o CameraFrame.o -o $(PROG) $(LIBS)



main.o: main.cpp ../../CaptureDevice.h ../../../Ganzheit/jpeg/jpeg.h
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp


CaptureDevice.o: ../../CaptureDevice.cpp ../../CaptureDevice.h ../../VideoControl.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../CaptureDevice.cpp


jpeg.o: ../../../Ganzheit/jpeg/jpeg.cpp ../../../Ganzheit/jpeg/jpeg.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../Ganzheit/jpeg/jpeg.cpp


VideoControl.o: ../../VideoControl.cpp ../../VideoControl.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../VideoControl.cpp


CameraFrame.o: ../../CameraFrame.cpp ../../CameraFrame.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../CameraFrame.cpp

clean:
	rm -f *.o $(PROG)

