all: tetris

tetris: tetris.o primlib.o
	/usr/local/gcc-4.0.2/bin/gcc -fbounds-checking -g $^ -o $@ -lm -lSDL -lpthread -lSDL_gfx

.c.o: 
	/usr/local/gcc-4.0.2/bin/gcc -fbounds-checking -g -Wall -pedantic -std=c99 -c -D_REENTRANT $<

primlib.o: primlib.c primlib.h

hanoi.o: tetris.c primlib.h

clean:
	-rm primlib.o tetris.o tetris
