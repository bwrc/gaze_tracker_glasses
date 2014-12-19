
CC=g++
CFLAGS=-c -g -Wall -pedantic
LIBS=-ljpeg -L/usr/local/src/OpenCV-2.3.1/build/debug/lib/ -lopencv_core -lopencv_highgui -lopencv_imgproc

INCLUDES=-I/usr/local/src/OpenCV-2.3.1/build/debug/include/


PROG=compress


all: $(PROG)

$(PROG): main.o MyTimer.o jpeg.o
	$(CC) $(LIBS) main.o MyTimer.o jpeg.o -o $(PROG)


main.o: main.cpp MyTimer.h jpeg.h
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp


MyTimer.o: MyTimer.cpp MyTimer.h
	$(CC) $(CFLAGS) $(INCLUDES) MyTimer.cpp


jpeg.o: jpeg.cpp jpeg.h
	$(CC) $(CFLAGS) $(INCLUDES) jpeg.cpp


clean:
	rm *.o $(PROG)

