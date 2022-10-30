#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h> 

#define LEVELS 5
#define CAPACITY 20
#define MAX_CARPARK 100
#define STORAGE_CAPACITY 7

typedef struct vehicle {
	/// The content of the vector.
	char plate[STORAGE_CAPACITY];

    // Entrance time 
    clock_t entranceTime;

    // Park time 
    clock_t parkTime;

    // Level
    int level;

    // Exit status 
    bool exitStatus;

    // Number of time passed LPR
    int LPRcount;
} vehicle_t;

typedef struct vehicleStorage {
	/// The current number of elements in the vector
	int size;

	/// The content of the vector.
	vehicle_t vehicle[MAX_CARPARK];
} vehicleStorage_t;

void storageInit(vehicleStorage_t *vehicleStorage);
void addCar(vehicleStorage_t *vehicleStorage, char *plate, clock_t entranceTime, clock_t parkTime, int level);
void removeCar(vehicleStorage_t* vehicleQueue, char *plate);
void printCarList(vehicleStorage_t* vehicleStorage);
int findIndex(vehicleStorage_t* vehicleStorage, char *plate);

// Initialise the storage
void storageInit(vehicleStorage_t *vehicleStorage){
    vehicleStorage->size = 0;
    for (int i = 0; i < MAX_CARPARK; i++){
        strcpy(vehicleStorage->vehicle[i].plate, "empty");
        vehicleStorage->vehicle[i].entranceTime = 0;
        vehicleStorage->vehicle[i].parkTime = 0;
        vehicleStorage->vehicle[i].exitStatus = false;
        vehicleStorage->vehicle[i].level = 1;
        vehicleStorage->vehicle[i].LPRcount = 0;
    }
}

// Add a vehicle to the storage
void addCar(vehicleStorage_t *vehicleStorage, char *plate, clock_t entranceTime, clock_t parkTime, int level){
    int old_size = vehicleStorage->size;
    memcpy(vehicleStorage->vehicle[old_size].plate, plate, 7);
    vehicleStorage->vehicle[old_size].entranceTime = entranceTime;
    vehicleStorage->vehicle[old_size].parkTime = parkTime;
    vehicleStorage->vehicle[old_size].level = level;
    vehicleStorage->vehicle[old_size].exitStatus = false;
    vehicleStorage->vehicle[old_size].LPRcount = 0;
    vehicleStorage->size = old_size + 1;
}

// Remove a vehicle based on its plate
void removeCar(vehicleStorage_t* vehicleStorage, char *plate){
    int old_size = vehicleStorage->size;
    vehicle_t old_vehicle[MAX_CARPARK];
    int loc;

    for (int i = 0; i < old_size; i++){
        memcpy(old_vehicle[i].plate, vehicleStorage->vehicle[i].plate, 7);
        old_vehicle[i].entranceTime = vehicleStorage->vehicle[i].entranceTime;
        old_vehicle[i].parkTime = vehicleStorage->vehicle[i].parkTime;
        old_vehicle[i].level = vehicleStorage->vehicle[i].level;
        old_vehicle[i].exitStatus = vehicleStorage->vehicle[i].exitStatus;
        old_vehicle[i].LPRcount = vehicleStorage->vehicle[i].LPRcount;
    }

    for (int i = 0; i < old_size; i++){
        if (memcmp(vehicleStorage->vehicle[i].plate, plate, 7) == 0){
            loc = i;
            break;
        }
    }

    for (int i = 0; i < loc; i++){
        memcpy(vehicleStorage->vehicle[i].plate, old_vehicle[i].plate,7);
        vehicleStorage->vehicle[i].entranceTime = old_vehicle[i].entranceTime;
        vehicleStorage->vehicle[i].parkTime = old_vehicle[i].parkTime;
        vehicleStorage->vehicle[i].level = old_vehicle[i].level;
        vehicleStorage->vehicle[i].exitStatus = old_vehicle[i].exitStatus;
        vehicleStorage->vehicle[i].LPRcount = old_vehicle[i].LPRcount;
    }

    for (int i = loc; i < old_size - 1; i++){
        memcpy(vehicleStorage->vehicle[i].plate, old_vehicle[i + 1].plate,7);
        vehicleStorage->vehicle[i].entranceTime = old_vehicle[i + 1].entranceTime;
        vehicleStorage->vehicle[i].parkTime = old_vehicle[i + 1].parkTime;
        vehicleStorage->vehicle[i].level = old_vehicle[i + 1].level;
        vehicleStorage->vehicle[i].exitStatus = old_vehicle[i + 1].exitStatus;
        vehicleStorage->vehicle[i].LPRcount = old_vehicle[i + 1].LPRcount;
    }

    vehicleStorage->size = old_size - 1;
}

// Find index of plate (from numberplate)
int findIndex(vehicleStorage_t* vehicleStorage, char *plate){
    int index;
    for (int i = 0; i < vehicleStorage->size; i++){
        if (memcmp(vehicleStorage->vehicle[i].plate, plate,7) == 0){
            index = i;
            return index;
        }
    }
    return 1;
}

// Print vehicles 
void printCarList(vehicleStorage_t* vehicleStorage){
    printf("NUMBER OF PLATES ARE: %d\n", vehicleStorage->size); 
    for (int i = 0; i < vehicleStorage->size; i++){
        printf("Plate number %d is: %s\n", i, vehicleStorage->vehicle[i].plate);
        printf("Entrance time of vehicle %d is: %ld\n", i, vehicleStorage->vehicle[i].entranceTime);
        printf("Park time of vehicle %d is: %ld\n", i, vehicleStorage->vehicle[i].parkTime);
        printf("Level of vehicle %d is: %d\n", i, vehicleStorage->vehicle[i].level);
        printf("Number of LRP count %d is: %d\n", i, vehicleStorage->vehicle[i].LPRcount);
    }
}

// int main(void){
//     vehicleStorage_t vehicleStorage;
//     storageInit(&vehicleStorage);
//     printf("Adding 4 plates...\n");
//     addCar(&vehicleStorage, "jactoe", 1000, 10, 1);
//     addCar(&vehicleStorage, "jactoa", 950, 10, 2);
//     addCar(&vehicleStorage, "jactor", 923, 10, 3);
//     addCar(&vehicleStorage, "penisl", 5, 10, 4);    
//     // printCarList(&vehicleStorage);
//     // printf("\n");

//     // printf("Removing a plate 'jactor'...\n");
//     // removeCar(&vehicleStorage, "jactor");
//     // printCarList(&vehicleStorage);
//     // printf("\n");
//     int index = findIndex(&vehicleStorage, "jactor");
//     printf("Index of plate before removal is: %d\n", index);


//     printf("Removing a plate 'jacktoa'...\n");
//     removeCar(&vehicleStorage, "jactoa");
//     printCarList(&vehicleStorage);
//     printf("\n");
//     index = findIndex(&vehicleStorage, "jactor");
//     printf("Index of plate before removal is: %d\n", index);
// }