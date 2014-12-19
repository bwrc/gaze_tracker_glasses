# build type
ISDEBUG=false

# compiler
CC=g++

# flags
CFLAGS:=-c -g -Wall -pedantic

# libraries
LIBS:=

INCLUDES = -I../



# determine the build type
ifeq ($(ISDEBUG), true)
	CFLAGS+=-O0
else
	CFLAGS+=-O2
endif

PROG = params


all: $(PROG)


$(PROG): main.o InputParser.o
	$(CC) main.o InputParser.o -o $(PROG) $(LIBS)


main.o: main.cpp ../InputParser.h
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp


InputParser.o: ../InputParser.cpp ../InputParser.h
	$(CC) $(CFLAGS) $(INCLUDES) ../InputParser.cpp



clean:
	rm -f *.o $(PROG)

