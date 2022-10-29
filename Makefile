CC = gcc
CFLAGS = -Wall -pedantic -pthread -lrt 
LDFLAGS =

all: simulator manager firealarm

simulator: simulator.c
	$(CC) -o simulator simulator.c $(CFLAGS)

manager: manager.c
	$(CC) -o manager manager.c $(CFLAGS)

firealarm: firealarm.c
	$(CC) -o firealarm firealarm.c $(CFLAGS)

clean:
	rm -f manager simulator firealarm *.o


