CC=gcc
CFLAGS=-c -O3

UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	LDFLAGS += -lm -framework OpenGL -lGLEW -lglfw3 -lportaudio
else
	LDFLAGS += -lm -lGL -lGLEW -lglfw -lportaudio
endif

install:
	$(CC) $(CFLAGS) *.c
	$(CC) *.o $(LDFLAGS) -o bounce
	rm -f *.o

uninstall:
	rm -f bounce
