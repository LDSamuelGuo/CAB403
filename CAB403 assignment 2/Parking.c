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

void plateInit(vehicleQueue_t *vehicleQueue);
void addPlate(vehicleQueue_t *vehicleQueue, char *plate);
void popPlate(vehicleQueue_t* vehicleQueue); 
void popRandom(vehicleQueue_t* vehicleQueue, int index); 
void printPlate(vehicleQueue_t* vehicleQueue);

// Initialise the queue
void plateInit(vehicleQueue_t *vehicleQueue){
    vehicleQueue->size = 0;
    for (int i = 0; i < MAX_CAPACITY; i++){
        strcpy(vehicleQueue->plateQueue[i], "empty");
    }
}

// Append plate to end of queue
void addPlate(vehicleQueue_t* vehicleQueue, char * plate){
    int old_size = vehicleQueue->size;
    strcpy(vehicleQueue->plateQueue[old_size], plate);
    vehicleQueue->size = old_size + 1;
}

// Remove first plate from queue
void popPlate(vehicleQueue_t* vehicleQueue){
    int old_size = vehicleQueue->size;
    char old_data[MAX_CAPACITY][STORAGE_CAPACITY];
    for (int i = 0; i < old_size; i++){
        strcpy(old_data[i], vehicleQueue->plateQueue[i]);
    }
    for (int i = 0; i < old_size - 1; i++){
        strcpy(vehicleQueue->plateQueue[i], old_data[i + 1]);
    }
    vehicleQueue->size = old_size - 1;
}

// Pop a plate at an index
void popRandom(vehicleQueue_t* vehicleQueue, int index){
    int old_size = vehicleQueue->size;
    char old_data[MAX_CAPACITY][STORAGE_CAPACITY];
    for (int i = 0; i < old_size; i++){
        strcpy(old_data[i], vehicleQueue->plateQueue[i]);
    }
    for (int i = 0; i < index; i++){
        strcpy(vehicleQueue->plateQueue[i], old_data[i]);
    }

    for (int i = index; i < old_size - 1; i++){
        strcpy(vehicleQueue->plateQueue[i], old_data[i + 1]);
    }
    vehicleQueue->size = old_size - 1;
}

// Print plates
void printPlate(vehicleQueue_t* vehicleQueue){
    printf("NUMBER OF PLATES ARE: %d\n", vehicleQueue->size); 
    for (int i = 0; i < vehicleQueue->size; i++){
        printf("Plate number %d is: %s\n", i, vehicleQueue->plateQueue[i]);
    }
}