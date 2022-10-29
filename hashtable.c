#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h> 

#define LEVELS 5
#define CAPACITY 20
#define MAX_CARPARK 100
#define STORAGE_CAPACITY 7

typedef struct vehicle {
	char plate[STORAGE_CAPACITY];
    clock_t entranceTime;
    clock_t parkTime;
    int level;
    bool exitStatus;
    int LPRcount;
} vehicle_t;

typedef struct htable {
	int size;
	vehicle_t vehicle[MAX_CARPARK];
} htable_t;


// Initialise the storage
bool htable_init(htable_t *htable){
    htable->size = 0;
    int i = 0;
    while (i < MAX_CARPARK)
    {
        strcpy(htable->vehicle[i].plate, "empty");
        htable->vehicle[i].entranceTime = 0;
        htable->vehicle[i].parkTime = 0;
        htable->vehicle[i].exitStatus = false;
        htable->vehicle[i].level = 1;
        htable->vehicle[i].LPRcount = 0;
        i++;
    }
}

// Find index of plate (from numberplate)
int findIndex(htable_t* htable, char *plate){
    int index, i=0 ;
    while (i < htable->size){
        if (memcmp(htable->vehicle[i].plate, plate,7) == 0){
            index = i;
            return index;
        }
        i++;
    }
    return 1;
}

// Print vehicles 
void printVehicleList(htable_t* htable){
    printf("NUMBER OF PLATES ARE: %d\n", htable->size); 
    int i = 0;
    for (i < htable->size){
        printf("Plate number %d is: %s\n", i, htable->vehicle[i].plate);
        printf("Entrance time of vehicle %d is: %ld\n", i, htable->vehicle[i].entranceTime);
        printf("Park time of vehicle %d is: %ld\n", i, htable->vehicle[i].parkTime);
        printf("Level of vehicle %d is: %d\n", i, htable->vehicle[i].level);
        printf("Number of LRP count %d is: %d\n", i, htable->vehicle[i].LPRcount);
        i++;
    }
}



// Add a vehicle to the storage
bool htable_add(htable_t *htable, char *plate, clock_t entranceTime, clock_t parkTime, int level){
    int old_size = htable->size;
    memcpy(htable->vehicle[old_size].plate, plate, 7);
    htable->vehicle[old_size].entranceTime = entranceTime;
    htable->vehicle[old_size].parkTime = parkTime;
    htable->vehicle[old_size].level = level;
    htable->vehicle[old_size].exitStatus = false;
    htable->vehicle[old_size].LPRcount = 0;
    htable->size = old_size + 1;
}

// Remove a vehicle based on its plate
void htable_delete(htable_t* htable, char *plate){
    int old_size = htable->size;
    vehicle_t old_vehicle[MAX_CARPARK];
    int base;

    for (int i = 0; i < old_size; i++){
        memcpy(old_vehicle[i].plate, htable->vehicle[i].plate, 7);
        old_vehicle[i].entranceTime = htable->vehicle[i].entranceTime;
        old_vehicle[i].parkTime = htable->vehicle[i].parkTime;
        old_vehicle[i].level = htable->vehicle[i].level;
        old_vehicle[i].exitStatus = htable->vehicle[i].exitStatus;
        old_vehicle[i].LPRcount = htable->vehicle[i].LPRcount;
    }

    for (int i = 0; i < old_size; i++){
        if (memcmp(htable->vehicle[i].plate, plate, 7) == 0){
            loc = i;
            break;
        }
    }

    for (int i = 0; i < loc; i++){
        memcpy(htable->vehicle[i].plate, old_vehicle[i].plate,7);
        htable->vehicle[i].entranceTime = old_vehicle[i].entranceTime;
        htable->vehicle[i].parkTime = old_vehicle[i].parkTime;
        htable->vehicle[i].level = old_vehicle[i].level;
        htable->vehicle[i].exitStatus = old_vehicle[i].exitStatus;
        htable->vehicle[i].LPRcount = old_vehicle[i].LPRcount;
    }

    for (int i = base; i < old_size - 1; i++){
        memcpy(htable->vehicle[i].plate, old_vehicle[i + 1].plate,7);
        htable->vehicle[i].entranceTime = old_vehicle[i + 1].entranceTime;
        htable->vehicle[i].parkTime = old_vehicle[i + 1].parkTime;
        htable->vehicle[i].level = old_vehicle[i + 1].level;
        htable->vehicle[i].exitStatus = old_vehicle[i + 1].exitStatus;
        htable->vehicle[i].LPRcount = old_vehicle[i + 1].LPRcount;
    }

    htable->size = old_size - 1;
}
