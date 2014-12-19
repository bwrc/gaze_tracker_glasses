# build type
ISDEBUG=true

# compiler
CC=g++

# flags
CFLAGS:=-c -g -Wall -pedantic

# libraries
LIBS := -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_calib3d -lm

# includes
INCLUDES:=	-I../../


# determine the build type
ifeq ($(ISDEBUG), true)
	INCLUDES+=-I/usr/local/src/OpenCV-2.4.0/build/debug/include/
	LIBS+=-L/usr/local/src/OpenCV-2.4.0/build/debug/lib
else
	INCLUDES+=-I/usr/local/src/OpenCV-2.4.0/build/release/include/
	LIBS+=-L/usr/local/src/OpenCV-2.4.0/build/release/lib
	CFLAGS+=-O2
endif


PROG = coord_system


all: $(PROG)


$(PROG): main.o Camera.o
	$(CC) main.o Camera.o -o $(PROG) $(LIBS)


main.o: main.cpp ../../Camera.h
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp


Camera.o: ../../Camera.cpp ../../Camera.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../Camera.cpp


clean:
	rm -f *.o $(PROG)

