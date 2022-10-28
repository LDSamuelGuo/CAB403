CC = gcc
CFLAGS = -Wall -pedantic -pthread -lrt 
LDFLAGS =

all: simulator manager fireAlarm

simulator: simulator.c
	$(CC) -o simulator simulator.c $(CFLAGS)

manager: manager.c
	$(CC) -o manager manager.c $(CFLAGS)

firealarm: firealarm.c
	$(CC) -o FireAlarm firealarm.c $(CFLAGS)

clean:
	rm -f manager simulator firealarm *.o


