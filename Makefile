LDFLAG=-pthread -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lxml2 -lglib-2.0 -L/usr/X11R6/lib -L/usr/local/lib
LIBS= -lARgsub -lARvideo -lARMulti -lAR -lpthread -lglut -lGLU -lGL -lXi -lX11 -lm -lBulletCollision -lBulletDynamics -lBulletSoftBody -lLinearMath
CFLAG= -O -pthread -I/usr/include/gstreamer-0.10 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I/usr/include/libxml2 -I/usr/X11R6/include -Ibullet/include

main: main.cpp model.o tiny_obj_loader.o physics.o
	g++ main.cpp model.cpp tiny_obj_loader.cpp physics.cpp -o main $(CFLAG) $(LDFLAG) $(LIBS)

model.o: model.cpp tiny_obj_loader.o
	g++ -c model.cpp -o model.o $(CFLAG)

tiny_obj_loader.o: tiny_obj_loader.cpp
	g++ -c tiny_obj_loader.cpp -o tiny_obj_loader.o $(CFLAG) 

physics.o: physics.cpp
	g++ -c physics.cpp -o physics.o $(CFLAG)


clean:
	rm -f *.o
	rm -f main
