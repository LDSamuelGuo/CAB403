CC = gcc
CFLAGS = -Wall -pedantic -pthread -lrt # Show all reasonable warnings
LDFLAGS =

all: simulator manager firealarm

simulator: simulator.c
	gcc -o simulator simulator.c $(CFLAGS)

manager: manager.c
	gcc -o manager manager.c $(CFLAGS)

firealarm: firealarm.c
	gcc -o firealarm firealarm.c $(CFLAGS)

clean:
	rm -f manager simulator firealarm *.o

.PHONY: all clean
