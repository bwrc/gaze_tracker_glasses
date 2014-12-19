
CC = g++

CFLAGS := -Wall -pedantic -g

PROG = group

INCLUDES =	-I/usr/local/src/OpenCV-2.3.1/build/debug/include/ \
			-I./../../

LIBS = -L/usr/local/src/OpenCV-2.3.1/build/debug/lib


# libraries
LIBS+=-lopencv_core -lopencv_highgui -lopencv_imgproc -lm


all: $(PROG)


$(PROG): main.o group.o
	$(CC) main.o group.o -o $(PROG)  $(LIBS)


main.o: main.cpp ../../group.h
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp


group.o: ../../group.cpp ../../group.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../group.cpp


clean:
	rm -f $(PROG) *.o

