#include <stdio.h>
#include <string.h>

#define MAX_CAPACITY 100
#define STORAGE_CAPACITY 7

typedef struct vehicleQueue {
	// The current number of elements in the vector
	int size;

	// The content of the vector.
	char plateQueue[MAX_CAPACITY][STORAGE_CAPACITY];
} vehicleQueue_t;


// Initialise the queue
void plateInit(vehicleQueue_t *vehicleQueue){
    vehicleQueue->size = 0;
    int i = 0;
    while (i < MAX_CAPACITY){
        strcpy(vehicleQueue->plateQueue[i], "empty");
        i++;
    }
}

// Append plate to end of queue
void addPlate(vehicleQueue_t* vehicleQueue, char * plate){
    int old_size = vehicleQueue->size;
    strcpy(vehicleQueue->plateQueue[old_size], plate);
    vehicleQueue->size = old_size + 1;
}

// Remove first plate from queue
void deletePlate(vehicleQueue_t* vehicleQueue){
    int old_size = vehicleQueue->size;
    char old_data[MAX_CAPACITY][STORAGE_CAPACITY];
    int i = 0;
    while (i < old_size)
    {
        strcpy(old_data[i], vehicleQueue->plateQueue[i]);
        i++;
    }
   while (i < old_size-1){
        strcpy(vehicleQueue->plateQueue[i], old_data[i + 1]);
        i++;
    }
    vehicleQueue->size = old_size - 1;
}

// Pop a plate at an index
void popRandom(vehicleQueue_t* vehicleQueue, int index){
    int old_size = vehicleQueue->size;
    char old_data[MAX_CAPACITY][STORAGE_CAPACITY];
    int i = 0;
    while (i < old_size){
        strcpy(old_data[i], vehicleQueue->plateQueue[i]);
        i++;
    }
    while (i < index){
        strcpy(vehicleQueue->plateQueue[i], old_data[i]);
        i++;
    }

    while (i < old_size - 1){
        strcpy(vehicleQueue->plateQueue[i], old_data[i + 1]);
        i++;
    }
    vehicleQueue->size = old_size - 1;
}

// Print plates
void printPlate(vehicleQueue_t* vehicleQueue){
    printf("NUMBER OF PLATES ARE: %d\n", vehicleQueue->size); 
    int i = 0;
    while (i < vehicleQueue->size)
    {
        printf("Plate number %d is: %s\n", i, vehicleQueue->plateQueue[i]);
        i++;
    }
}

