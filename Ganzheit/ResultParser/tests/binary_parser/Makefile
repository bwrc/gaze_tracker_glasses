
CC = g++

CFLAGS := -Wall -pedantic -g

PROG = parser

INCLUDES = -I/usr/local/src/OpenCV-2.3.1/build/debug/include/ \
		   -I../../


LIBS = -L/usr/local/src/OpenCV-2.3.1/build/debug/lib/



all: $(PROG)


$(PROG): main.o BinaryResultParser.o ResultData.o
	$(CC) main.o BinaryResultParser.o ResultData.o -o $(PROG)  $(LIBS)


main.o: main.cpp ../../BinaryResultParser.h
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp


BinaryResultParser.o: ../../BinaryResultParser.cpp ../../BinaryResultParser.h ../../ResultData.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../BinaryResultParser.cpp


ResultData.o: ../../ResultData.cpp ../../ResultData.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../ResultData.cpp


clean:
	rm -f $(PROG) *.o

