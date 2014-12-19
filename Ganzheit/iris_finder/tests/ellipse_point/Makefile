# build type
ISDEBUG=true

# compiler
CC=g++

# flags
CFLAGS:=-c -Wall -pedantic

# libraries
LIBS:= -lopencv_core -lopencv_highgui -lopencv_calib3d -lopencv_imgproc -lopencv_features2d -lm `sdl-config --libs`

# includes
INCLUDES:= -I../../../GazeTracker/ellipse


# determine the build type
ifeq ($(ISDEBUG), true)
	INCLUDES+=-I/usr/local/src/OpenCV-2.4.0/build/debug/include/
	LIBS+=-L/usr/local/src/OpenCV-2.4.0/build/debug/lib
	CFLAGS+=-g
else
	INCLUDES+=-I/usr/local/src/OpenCV-2.4.0/build/release/include/
	LIBS+=-L/usr/local/src/OpenCV-2.4.0/build/release/lib
	CFLAGS+=-O2
endif


PROG = ellipse

OBJECTS = main.o ellipse.o


all: $(PROG)


$(PROG): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(PROG) $(LIBS)


main.o: main.cpp
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp

ellipse.o: ../../../GazeTracker/ellipse/ellipse.cpp ../../../GazeTracker/ellipse/ellipse.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../GazeTracker/ellipse/ellipse.cpp



clean:
	rm -f *.o $(PROG)

