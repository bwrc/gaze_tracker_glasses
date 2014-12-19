# I am a comment, and I want to say that the variable CC will be
# the compiler to use.
CC=g++
# Hey!, I am comment number 2. I want to say that CFLAGS will be the
# options I'll pass to the compiler.
CFLAGS=-c -g -static
LIBS=
INCLUDES=-I../../../socket_communication/

PROG = client


all: $(PROG)


$(PROG): main.o Client.o Communicator.o
	$(CC) main.o Client.o Communicator.o -o $(PROG) $(LIBS)


main.o: main.cpp ../../../socket_communication/Client.h ../../../socket_communication/Communicator.h
	$(CC) $(CFLAGS) $(INCLUDES) main.cpp


Communicator.o: ../../../socket_communication/Communicator.cpp ../../../socket_communication/Communicator.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../socket_communication/Communicator.cpp


Client.o: ../../../socket_communication/Client.cpp ../../../socket_communication/Client.h
	$(CC) $(CFLAGS) $(INCLUDES) ../../../socket_communication/Client.cpp


clean:
	rm -f *.o $(PROG)


