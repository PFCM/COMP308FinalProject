CC = g++
CFLAGS = -Wall -Wextra -pedantic -std=c++11
LPATH = -L/usr/X11/lib
IPATH = -I/usr/X11/include
LIBS = -lm -lGL -lGLU -lglut

all: release

release: CFLAGS += -O3
release: main.o
	$(CC) -o $@ $^ $(LIBS) $(LPATH)

debug: CFLAGS += -g
debug: main.o
	$(CC) -o $@ $^ $(LIBS) $(LPATH)

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $^ $(IPATH)