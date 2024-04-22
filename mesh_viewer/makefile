TARGET = view_mesh

INCLUDES = -Iinclude/ \
		   -I../include/

LINKS = -lglfw \
-L/usr/local/lib \

SRC     := ./src
SRCS    := $(wildcard $(SRC)/*.cpp) $(wildcard ../src/*.cpp) 
OBJS    := $(patsubst %.cpp,%.o,$(SRCS))
OBJS	:= $(OBJS) src/glad.c 
$(info $$OBJS is [${OBJS}])

CFLAGS = $(INCLUDES) -Wl,-rpath,/usr/local/lib -lstdc++ -std=c++1z -pthread $(LINKS) -O3
#-fsanitize=address

clean : 
	-rm src/*.o
	-rm ../src/*.o
	-rm main.o
	echo Clean done
	
all : $(TARGET)
	chmod +x view_mesh
	echo All done

$(TARGET) : $(OBJS) main.o
	g++ -g -o $@ $^ $(CFLAGS) $(BULLET_LINKS)

%.o : %.cpp
	g++ -g -o $@ -c $< $(CFLAGS)

%.o : %.c
	gcc -g -o $@ -c $< $(CFLAGS)