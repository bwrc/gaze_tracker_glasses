
CC = g++

CFLAGS := -Wall -pedantic -g

PROG = ellipse

INCLUDES = -I/usr/local/OpenCV-2.3.1/build/debug/include/
LIBS = -L/usr/local/OpenCV-2.3.1/build/debug/lib


# libraries
LIBS+=-lopencv_core -lopencv_highgui -lm


all: $(PROG)

$(PROG): main.o
	$(CC) main.o -o $(PROG)  $(LIBS)


main.o: main.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp

clean:
	rm -f $(PROG) *.o

