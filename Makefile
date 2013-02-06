CC=g++
LDFLAGS  += -L. -L/usr/local/lib -L/opt/vc/lib -L/opt/vc/src/hello_pi/libs/ilclient
LIBS    +=  -lbcm_host -lpthread -lGLESv2 -lEGL -lopenmaxil -lilclient -lvchiq_arm -lrt -lcamwire -lm -ldc1394 -lraw1394
CPPFLAGS +=-I.
CPPFLAGS += -I/usr/local/include -I/opt/vc/include -I/opt/vc/include/interface/vcos/pthreads -I/opt/vc/src/hello_pi/libs/ilclient 
CPPFLAGS += -Wall -g
CPPFLAGS += -DLINUX
CPPFLAGS += -Wall -O3
OBJECTS = main.o PointGreyCam.o GPU_Encode.o convert_colourspace.o
EXECUTABLE=GPU-Encode
all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)

%.o: %.cpp
	$(CC) -c  $< $(CPPFLAGS)

clean:
	rm -f *~ *.o *.d $(EXECUTABLES)
