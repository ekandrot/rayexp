CC = g++
COMPILER_FLAGS = `pkg-config libpng --cflags` -Wall -pthread -std=c++14 -c -O3
LINKER_FLAGS = `pkg-config libpng --libs` -lSDL2
OBJS = ray.o world.o png_wrapper.o
TARGET = rayexp

ifeq ($(OS),Windows_NT)
	CFLAGS += -D WIN32
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Linux)
		CFLAGS += -D LINUX
	endif
	ifeq ($(UNAME_S), Darwin)
		CFLAGS += -D OSX
		LFLAGS += -framework OpenGL -framework GLUT
	endif
endif


#This is the target that compiles our executable
all : ${TARGET}

clean :
	rm ${OBJS} ${TARGET}

${TARGET} :	$(OBJS)
	$(CC) $(LINKER_FLAGS) $(OBJS) -o $@

world.o : world.cpp world.h maths.h coords.h
	$(CC) $< $(COMPILER_FLAGS) -o $@

png_wrapper.o : png_wrapper.cpp png_wrapper.h
	$(CC) $< $(COMPILER_FLAGS) -o $@

ray.o : ray.cpp maths.h png_wrapper.h coords.h world.h scheduler.h
	$(CC) $< $(COMPILER_FLAGS) -o $@

