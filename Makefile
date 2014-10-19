CC = g++
CFLAGS = -Wall -Wextra -pedantic -std=c++11
LPATH = -L/usr/X11/lib -L/usr/pkg/lib
IPATH = -I/usr/X11/include -L/usr/package/include
LIBS = -lm -lGL -lGLU -lglut

all: release

release: CFLAGS += -O3
release: main.o quaternion.o
	$(CC) -o smokeparticles $^ $(LIBS) $(LPATH)

debug: CFLAGS += -g
debug: main.o quaternion.o
	$(CC) -o $@ $^ $(LIBS) $(LPATH)

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $^ $(IPATH)
