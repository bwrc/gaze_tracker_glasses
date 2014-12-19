# I am a comment, and I want to say that the variable CC will be
# the compiler to use.
CC=g++
# Hey!, I am comment number 2. I want to say that CFLAGS will be the
# options I'll pass to the compiler.
CFLAGS=-c -g -static
LIBS= -L/usr/local/src/OpenCV-2.3.1/build/debug/lib -lopencv_core -lopencv_highgui -lopencv_calib3d -lopencv_imgproc -lm
INCLUDES=-I../../../socket_communication/ -I/usr/local/src/OpenCV-2.3.1/build/debug/include/

PROG = server


all: $(PROG)


$(PROG): main.o Server.o Communicator.o
	$(CC) main.o Server.o Communicator.o -o $(PROG) $(LIBS)


main.o: main.cpp ../../../socket_communication/Server.h
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp


Communicator.o: ../../../socket_communication/Communicator.cpp ../../../socket_communication/Communicator.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../socket_communication/Communicator.cpp


Server.o: ../../../socket_communication/Server.cpp ../../../socket_communication/Server.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../socket_communication/Server.cpp


clean:
	rm -f *.o $(PROG)


