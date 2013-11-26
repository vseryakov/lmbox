CFLAGS=-g -Wall -fPIC `vlc-config --cflags`
LDFLAGS=-shared
LIBS=-ltcl8.4 -L/usr/lib/vlc `vlc-config --libs vlc external`

all: tclvlc.so

tclvlc.so: tclvlc.o
	gcc $(LDFLAGS) -o tclvlc.so tclvlc.o $(LIBS)

tclvlc.o: tclvlc.c
	gcc $(CFLAGS) -c tclvlc.c

clean:
	-rm *.so *~ *.o

