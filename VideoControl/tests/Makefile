CPP		:=	g++
LNK		:=	g++

CPPFLAGS	:= -pedantic -Wall -W -g -I../

all:
	$(CPP) -c $(CPPFLAGS) main.cpp
	$(CPP) -c $(CPPFLAGS) ../VideoControl.cpp
	$(LNK) *.o -o video_test

clean:
	$(RM) video_test *.o
