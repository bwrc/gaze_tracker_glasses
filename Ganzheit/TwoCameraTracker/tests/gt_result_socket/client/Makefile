# I am a comment, and I want to say that the variable CC will be
# the compiler to use.
CC=g++
# Hey!, I am comment number 2. I want to say that CFLAGS will be the
# options I'll pass to the compiler.
CFLAGS=-c -g -static
LIBS= -L/usr/local/src/OpenCV-2.3.1/build/debug/lib -lopencv_core -lopencv_highgui -lopencv_calib3d -lopencv_imgproc -lm
INCLUDES=	-I../../../												\
			-I/usr/local/src/OpenCV-2.3.1/build/debug/include/		\
			-I../../../../ResultParser/								\
			-I../../../socket_communication/

PROG = client


all: $(PROG)


$(PROG): main.o Client.o ResultData.o Communicator.o BinaryResultParser.o
	$(CC) main.o Client.o ResultData.o Communicator.o BinaryResultParser.o -o $(PROG) $(LIBS)


main.o: main.cpp ../../../socket_communication/Client.h
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp


Communicator.o: ../../../socket_communication/Communicator.cpp ../../../socket_communication/Communicator.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../socket_communication/Communicator.cpp

Client.o: ../../../socket_communication/Client.cpp ../../../socket_communication/Client.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../socket_communication/Client.cpp


BinaryResultParser.o: ../../../../ResultParser/BinaryResultParser.cpp ../../../../ResultParser/BinaryResultParser.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../ResultParser/BinaryResultParser.cpp


ResultData.o: ../../../../ResultParser/ResultData.cpp ../../../../ResultParser/ResultData.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../../ResultParser/ResultData.cpp


clean:
	rm -f *.o $(PROG)


