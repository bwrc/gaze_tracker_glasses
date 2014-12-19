
CC = g++

CFLAGS := -Wall -pedantic -g

PROG = pattern

OPENCV_DIR=/usr/local/src/OpenCV-2.4.0/


INCLUDES = -I$(OPENCV_DIR)build/debug/include/ \
	   -I./../../
LIBS = -L$(OPENCV_DIR)build/debug/lib


# libraries
LIBS+=-lopencv_core -lopencv_highgui -lopencv_imgproc -lm


all: $(PROG)

$(PROG): main.o Pattern.o Pupil.o group.o
	$(CC) main.o Pattern.o Pupil.o group.o -o $(PROG)  $(LIBS)


main.o: main.cpp Pattern.h Pupil.h
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp


Pattern.o: Pattern.cpp Pattern.h ../../group.h
	$(CC) $(CFLAGS) $(INCLUDES) -c Pattern.cpp


Pupil.o: Pupil.cpp Pupil.h
	$(CC) $(CFLAGS) $(INCLUDES) -c Pupil.cpp


group.o: ../../group.cpp ../../group.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../group.cpp


clean:
	rm -f $(PROG) *.o

