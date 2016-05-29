install:
	gcc -c *.c
	gcc *.o -lm -lGL -lGLEW -lglfw -lportaudio -o bounce
	rm -f *.o

uninstall:
	rm -f bounce