LDFLAG=-pthread -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lxml2 -lglib-2.0 -L/usr/X11R6/lib -L/usr/local/lib
LIBS= -lARgsub -lARvideo -lAR -lpthread -lglut -lGLU -lGL -lXi -lX11 -lm
CFLAG= -O -pthread -I/usr/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/libxml2 -I/usr/X11R6/include 

all: main.o
	g++ -o main main.o $(LDFLAG) $(LIBS)

main.o: main.cpp
	g++ -c $(CFLAG) main.cpp

clean:
	rm -f *.o
	rm -f main

allclean:
	rm -f *.o
	rm -f simpleTest
	rm -f Makefile
