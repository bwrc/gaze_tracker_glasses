
CC = g++

CFLAGS := -Wall -pedantic -g

PROG = parser

INCLUDES = -I/usr/local/src/OpenCV-2.3.1/build/debug/include/ \
		   -I../../


LIBS = -L/usr/local/src/OpenCV-2.3.1/build/debug/lib/



all: $(PROG)


$(PROG): main.o ResultParser.o ResultData.o
	$(CC) main.o ResultParser.o ResultData.o -o $(PROG)  $(LIBS)


main.o: main.cpp ../../ResultParser.h
	$(CC) $(CFLAGS) $(INCLUDES) -c main.cpp


ResultParser.o: ../../ResultParser.cpp ../../ResultParser.h ../../ResultData.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../ResultParser.cpp


ResultData.o: ../../ResultData.cpp ../../ResultData.h
	$(CC) $(CFLAGS) $(INCLUDES) -c ../../ResultData.cpp


clean:
	rm -f $(PROG) *.o

