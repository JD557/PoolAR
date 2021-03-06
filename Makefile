LDFLAG=-pthread -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lxml2 -lglib-2.0 -L/usr/X11R6/lib -L/usr/local/lib
LIBS= -lARgsub -lARvideo -lARMulti -lAR -lpthread -lglut -lGLU -lGL -lGLEW -lXi -lX11 -lm -lBulletSoftBody -lBulletDynamics -lBulletCollision -lLinearMath
CFLAG= -std=c++11 -fopenmp -pg -O -pthread -I/usr/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/libxml2 -I/usr/X11R6/include -Ibullet/include

main: main.cpp model.o tiny_obj_loader.o physics.o imgproc.o light.o vectorMath.o
	g++ main.cpp model.cpp tiny_obj_loader.cpp physics.cpp light.cpp imgproc.cpp vectorMath.cpp -o main $(CFLAG) $(LDFLAG) $(LIBS)

model.o: model.cpp tiny_obj_loader.o
	g++ -c model.cpp -o model.o $(CFLAG)

imgproc.o: imgproc.cpp
	g++ -c imgproc.cpp -o imgproc.o $(CFLAG)

tiny_obj_loader.o: tiny_obj_loader.cpp
	g++ -c tiny_obj_loader.cpp -o tiny_obj_loader.o $(CFLAG) 

physics.o: physics.cpp
	g++ -c physics.cpp -o physics.o $(CFLAG)

light.o: light.cpp
	g++ -c light.cpp -o light.o $(CFLAG)

vectorMath.o: vectorMath.cpp
	g++ -c vectorMath.cpp -o vectorMath.o $(CFLAG)

clean:
	rm -f *.o
	rm -f main
