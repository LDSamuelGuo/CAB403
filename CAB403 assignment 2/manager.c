// -------------------------------------------- HEADER -------------------------------------------- //
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include "sharedmemorymethod.c"
#include "Parking.c"
#include "vehicleStorage.c"

#define SHARE_NAME "PARKING"

// ------------------------------------ FUNCTION DECLERATIONS ------------------------------------- // 
void *entranceSimulate(void *arg);
void *entranceLPR(void *arg);
void *levelLPR(void *arg);
void *levelController(void *arg);
void *exitLPR(void *arg);
void *exitController(void *arg);
void *checkTimes(void *arg);
void *entranceBoomgate(void *arg);
void *exitBoomgate(void *arg);
void *statusDisplay(void *args);

double generateRandom(int lower, int upper);
void createThreads(void);
void generateBill(char*);
void readFile(char *filename);
void evactuationSequence(void);
void fireSignal(void);
void checkFire(void);

// --------------------------------------- PUBLIC VARIABLES --------------------------------------- // 
shared_memory_t shm;
vehicleQueue_t levelQueue[LEVELS];
vehicleQueue_t exitQueue[EXITS];
vehicleStorage_t vehicleStorage;

pthread_mutex_t levelQueueMutex[LEVELS];
pthread_mutex_t exitQueueMutex[EXITS];
pthread_mutex_t vehicleStorageMutex;
pthread_mutex_t levelCapacityMutex[LEVELS];

int levelCapacity[LEVELS];
char allowedPlates[100][7];
char statusDisp[ENTRANCES];
char exitDisplay[EXITS];
char entrDisplay[ENTRANCES];
double bill;
double moneyEarned;
int fire;

// --------------------------------------------- MAIN --------------------------------------------- // 
int main()
{    
    // Initialise random seed
    time_t t;
    srand((unsigned) time(&t));

    // Initialises all level queues
    for (int i = 0; i < LEVELS; i++){
        levelCapacity[i] = 0;
    }
    moneyEarned = 0;
    fire = 0;

    // Read the number plates 
    readFile("plates.txt");

    // Mount shared memory
    create_shared_object_R(&shm, SHARE_NAME);
    
    // Create and join threads
    createThreads();
}

// --------------------------------------- HELPER FUNCTUONS --------------------------------------- // 

void *entranceLPR(void *arg){
    int i = *(int*) arg;
    while (!fire){ 
        // Empty LPR
        pthread_mutex_lock(&shm.data->entrance[i].LPRSensor.LPRmutex);
        strcpy(shm.data->entrance[i].LPRSensor.plate, "000000");
        pthread_mutex_unlock(&shm.data->entrance[i].LPRSensor.LPRmutex);

        // When entrance LPR is free, signal the simulator thread asking for a plate
        pthread_cond_signal(&shm.data->entrance[i].LPRSensor.LPRcond);

        // Wait for plate
        pthread_mutex_lock(&shm.data->entrance[i].LPRSensor.LPRmutex);
        while (!strcmp(shm.data->entrance[i].LPRSensor.plate, "000000")){
            pthread_cond_wait(&shm.data->entrance[i].LPRSensor.LPRcond, &shm.data->entrance[i].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->entrance[i].LPRSensor.LPRmutex);
        // Plate recieved 

        // Information sign stuff here. For now, send to level 1.
        bool allowed = false;
        char toDisplay;

        // Check if allowed in parking lot
        for (int j = 0; j < 100; j++) {
            pthread_mutex_lock(&shm.data->entrance[i].LPRSensor.LPRmutex);
            int returnVal = strcmp(shm.data->entrance[i].LPRSensor.plate, allowedPlates[j]);
            pthread_mutex_unlock(&shm.data->entrance[i].LPRSensor.LPRmutex);
            if (returnVal != 0) { 
                toDisplay = 'X';
                allowed = false;
            }    
            else {          
                allowed = true;
                break;
            }
            
        }

        // Check to see if vehiclepark is full
        if (allowed) {
            pthread_mutex_lock(&vehicleStorageMutex);
            int currentCapacity = vehicleStorage.size;
            pthread_mutex_unlock(&vehicleStorageMutex);
            if (currentCapacity >= CAPACITY * LEVELS){
                toDisplay = 'F';
            }
            else{
                toDisplay = generateRandom(0,4) + '0';
            }
        }

        // Set information sign display to 'toDisplay' to show its ready
        pthread_mutex_lock(&shm.data->entrance[i].informationSign.ISmutex);
        shm.data->entrance[i].informationSign.display = toDisplay;
        pthread_mutex_unlock(&shm.data->entrance[i].informationSign.ISmutex);
        
        // Despawn vehicle if not allowed in
        if (toDisplay != 'F' && toDisplay != 'X'){
            // Boomgates 
            pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
            entrDisplay[i] = shm.data->entrance[i].gate.status;
            pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);

            // If not open, wait for boomgate sequence
            if (entrDisplay[i] != 'O')
            {
                pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
                shm.data->entrance[i].gate.status = 'R';
                pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);

                pthread_cond_signal(&shm.data->entrance[i].gate.gatecond);
                pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
                while(shm.data->entrance[i].gate.status != 'O') {
                    pthread_cond_wait(&shm.data->entrance[i].gate.gatecond, &shm.data->entrance[i].gate.gatemutex);
                }
                pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);

            }
            // Add plate to level queue
            pthread_mutex_lock(&levelQueueMutex[i]);
            pthread_mutex_lock(&shm.data->entrance[i].LPRSensor.LPRmutex);
            addPlate(&levelQueue[i], shm.data->entrance[i].LPRSensor.plate);
            pthread_mutex_unlock(&shm.data->entrance[i].LPRSensor.LPRmutex);
            pthread_mutex_unlock(&levelQueueMutex[i]);

            // Generate random park time
            clock_t parkTime = (clock_t) generateRandom(100,10000) * 1000;
            printf("NLPR - Time spent parked will be %0.2f\n", (double)parkTime/CLOCKS_PER_SEC);

            // Log vehicle
            pthread_mutex_lock(&vehicleStorageMutex);
            pthread_mutex_lock(&shm.data->entrance[i].LPRSensor.LPRmutex);
            addCar(&vehicleStorage, shm.data->entrance[i].LPRSensor.plate, clock(), parkTime,i);
            pthread_mutex_unlock(&shm.data->entrance[i].LPRSensor.LPRmutex);
            pthread_mutex_unlock(&vehicleStorageMutex);
        }
    }
    return 0;
}

void *levelLPR(void *arg){
    int i = *(int*) arg;
    while (!fire){ 
        while(levelQueue[i].size <= 0);
        // Empty LPR
        pthread_mutex_lock(&shm.data->level[i].LPRSensor.LPRmutex);
        strcpy(shm.data->level[i].LPRSensor.plate, "000000");
        pthread_mutex_unlock(&shm.data->level[i].LPRSensor.LPRmutex);

        // When level LPR is free, signal the simulator thread asking for a plate
        pthread_cond_signal(&shm.data->level[i].LPRSensor.LPRcond);

        pthread_mutex_lock(&shm.data->level[i].LPRSensor.LPRmutex);
        while (!strcmp(shm.data->level[i].LPRSensor.plate, "000000")){
            pthread_cond_wait(&shm.data->level[i].LPRSensor.LPRcond, &shm.data->level[i].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->level[i].LPRSensor.LPRmutex);
        // Plate recieved 

        // Find plate in storage
        pthread_mutex_lock(&vehicleStorageMutex);
        pthread_mutex_lock(&shm.data->level[i].LPRSensor.LPRmutex);
        char plate[7];
        strcpy(plate,shm.data->level[i].LPRSensor.plate);
        while (findIndex(&vehicleStorage,plate) == -1);
        int returnVal = vehicleStorage.vehicle[findIndex(&vehicleStorage,plate)].LPRcount;
        pthread_mutex_unlock(&shm.data->level[i].LPRSensor.LPRmutex);
        pthread_mutex_unlock(&vehicleStorageMutex);

        // Check if its the vehicles first or second time passing through level LPR
        if (returnVal == 0) {
            // Check to see if there is still room
            pthread_mutex_lock(&levelCapacityMutex[i]);
            int currentLevelCap = levelCapacity[i];
            pthread_mutex_unlock(&levelCapacityMutex[i]);

            if (currentLevelCap < CAPACITY) {
                // Wait 10ms to drive to parking spot before triggering LPR
                usleep(10000);

                // Send vehicle to parking spot (Add to level counter and capacity)
                pthread_mutex_lock(&vehicleStorageMutex);
                pthread_mutex_lock(&shm.data->level[i].LPRSensor.LPRmutex);
                while (findIndex(&vehicleStorage,plate) == -1);
                vehicleStorage.vehicle[findIndex(&vehicleStorage,shm.data->level[i].LPRSensor.plate)].LPRcount = 1;
                pthread_mutex_unlock(&shm.data->level[i].LPRSensor.LPRmutex);
                pthread_mutex_unlock(&vehicleStorageMutex);
                pthread_mutex_lock(&levelCapacityMutex[i]);
                levelCapacity[i]++;
                pthread_mutex_unlock(&levelCapacityMutex[i]);
            }
            else {
                // As there is no more information signs, the vehicle will wander around the park until it fits somewhere
                pthread_mutex_lock(&levelQueueMutex[i]);
                addPlate(&levelQueue[(int)generateRandom(0,4)],plate);
                pthread_mutex_unlock(&levelQueueMutex[i]);
            }
        }
        else {
            // Second time seen by LPR -> add to exit
            pthread_mutex_lock(&exitQueueMutex[i]);
            pthread_mutex_lock(&shm.data->level[i].LPRSensor.LPRmutex);
            addPlate(&exitQueue[i],shm.data->level[i].LPRSensor.plate);
            pthread_mutex_unlock(&shm.data->level[i].LPRSensor.LPRmutex);
            pthread_mutex_unlock(&exitQueueMutex[i]);
            // Decrease capacity
            pthread_mutex_lock(&levelCapacityMutex[i]);
            if (levelCapacity[i] != 0){
                levelCapacity[i]--;
            } 
            pthread_mutex_unlock(&levelCapacityMutex[i]);
        }
    }
    return 0;
}

void *levelController(void *arg){
    int i = *(int*) arg;
    while (!fire){ 
        // Wait for manager LPR thread to signal that LPR is free
        pthread_mutex_lock(&shm.data->level[i].LPRSensor.LPRmutex);
        while (strcmp(shm.data->level[i].LPRSensor.plate, "000000")){ 
            pthread_cond_wait(&shm.data->level[i].LPRSensor.LPRcond, &shm.data->level[i].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->level[i].LPRSensor.LPRmutex);
        // LRP is now clear

        // Make sure plate is in queue
        while(levelQueue[i].size <= 0);

        // Copy a plate into memory
        pthread_mutex_lock(&shm.data->level[i].LPRSensor.LPRmutex);
        pthread_mutex_lock(&levelQueueMutex[i]);
        strcpy(shm.data->level[i].LPRSensor.plate, levelQueue[i].plateQueue[0]);
        popPlate(&levelQueue[i]);
        pthread_mutex_unlock(&shm.data->level[i].LPRSensor.LPRmutex);
        pthread_mutex_unlock(&levelQueueMutex[i]);

        // Signal manager thread with new plate
        pthread_cond_signal(&shm.data->level[i].LPRSensor.LPRcond); 
    }
    return 0;
}


void *exitLPR(void *arg){
    int i = *(int*) arg;
    while (!fire){ 
        while(exitQueue[i].size <= 0);
        // Empty LPR
        pthread_mutex_lock(&shm.data->exit[i].LPRSensor.LPRmutex);
        strcpy(shm.data->exit[i].LPRSensor.plate, "000000");
        pthread_mutex_unlock(&shm.data->exit[i].LPRSensor.LPRmutex);

        // When exit LPR is free, signal the simulator thread asking for a plate
        pthread_cond_signal(&shm.data->exit[i].LPRSensor.LPRcond);

        pthread_mutex_lock(&shm.data->exit[i].LPRSensor.LPRmutex);
        while (!strcmp(shm.data->exit[i].LPRSensor.plate, "000000")){
            pthread_cond_wait(&shm.data->exit[i].LPRSensor.LPRcond, &shm.data->exit[i].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->exit[i].LPRSensor.LPRmutex);
        // Plate recieved

        // Boomgate status 
        pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
        exitDisplay[i] = shm.data->exit[i].gate.status;
        pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);

        if (exitDisplay[i] != 'O')
        {   
            pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
            shm.data->exit[i].gate.status = 'R';
            pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);

            pthread_cond_signal(&shm.data->exit[i].gate.gatecond);
            pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
            while(shm.data->exit[i].gate.status != 'O') {
                pthread_cond_wait(&shm.data->exit[i].gate.gatecond, &shm.data->exit[i].gate.gatemutex);
            }
            pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);
        }

        // Bill the vehicle
        pthread_mutex_lock(&vehicleStorageMutex);
        pthread_mutex_lock(&shm.data->exit[i].LPRSensor.LPRmutex);
        generateBill(shm.data->exit[i].LPRSensor.plate); 

        // Remove from system 
        removeCar(&vehicleStorage, shm.data->exit[i].LPRSensor.plate);
        pthread_mutex_unlock(&shm.data->exit[i].LPRSensor.LPRmutex);
        pthread_mutex_unlock(&vehicleStorageMutex);
    }
    return 0;
}

void *exitController(void *arg){
    int i = *(int*) arg;
    while (!fire){ 
        // Wait for exit LPR thread to signal that LPR is free
        pthread_mutex_lock(&shm.data->exit[i].LPRSensor.LPRmutex);
        while (strcmp(shm.data->exit[i].LPRSensor.plate, "000000")){ 
            pthread_cond_wait(&shm.data->exit[i].LPRSensor.LPRcond, &shm.data->exit[i].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->exit[i].LPRSensor.LPRmutex);
        // LRP is clear 

        // Make sure plate is in queue
        while(exitQueue[i].size <= 0);

        // Copy a plate into memory
        pthread_mutex_lock(&shm.data->exit[i].LPRSensor.LPRmutex);
        pthread_mutex_lock(&exitQueueMutex[i]);
        strcpy(shm.data->exit[i].LPRSensor.plate, exitQueue[i].plateQueue[0]);
        pthread_mutex_unlock(&shm.data->exit[i].LPRSensor.LPRmutex);
        
        // Remove from exit queue
        popPlate(&exitQueue[i]);
        pthread_mutex_unlock(&exitQueueMutex[i]);

        // Wait 10ms before triggering exit LPR
        usleep(10000);

        // Signal manager thread with new plate
        pthread_cond_signal(&shm.data->exit[i].LPRSensor.LPRcond);
    }
    return 0;
}

void *checkTimes(void *arg){
    int i = *(int*) arg;
    while (!fire){ 
        while (vehicleStorage.size <= 0);
        // Loop through all parked vehicles
        for (i = 0; i < vehicleStorage.size; i++){
            if (vehicleStorage.vehicle[i].exitStatus != true && (double) vehicleStorage.vehicle[i].entranceTime + vehicleStorage.vehicle[i].parkTime <= (double) clock()){
                vehicleStorage.vehicle[i].exitStatus = true;
                // Remove from parking lot
                pthread_mutex_lock(&levelQueueMutex[vehicleStorage.vehicle[i].level]);
                addPlate(&levelQueue[vehicleStorage.vehicle[i].level],vehicleStorage.vehicle[i].plate);
                pthread_mutex_unlock(&levelQueueMutex[vehicleStorage.vehicle[i].level]);
            }
        }
        // Check to see if fire on any level
        checkFire();
    }
    // FIRE -> set all gates to open
    fireSignal();
    return 0;
}

void *entranceBoomgate(void *arg) {
    int i = *(int*) arg;
    while (!fire){ 
        // Waiting for status of boom gate to change
        pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
        while(shm.data->entrance[i].gate.status == 'C') {
            pthread_cond_wait(&shm.data->entrance[i].gate.gatecond, &shm.data->entrance[i].gate.gatemutex);
        }
        pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);

        // Set gate to raising (10 ms)
        pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
        shm.data->entrance[i].gate.status = 'R';
        pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);
        usleep(10000);

        // Set gate to open
        pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
        shm.data->entrance[i].gate.status = 'O';
        pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);
        pthread_cond_signal(&shm.data->entrance[i].gate.gatecond);

        // Wait for vehicle to automatically close gate
        usleep(20000);
        
        // Set gate to lowering (10ms)
        pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
        shm.data->entrance[i].gate.status = 'L';
        pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);
        usleep(10000);

        // Set gate to closed   
        pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
        shm.data->entrance[i].gate.status = 'C';
        pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);
    }
    // FIRE -> set all gates to open
    fireSignal();
    return 0;
}

void *exitBoomgate(void *arg) {
    int i = *(int*) arg;
    while (!fire){ 
        // Waiting for status of boom gate to change 
        pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
        while(shm.data->exit[i].gate.status == 'C') {
            pthread_cond_wait(&shm.data->exit[i].gate.gatecond, &shm.data->exit[i].gate.gatemutex);
        }
        pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);
    
        // Set gate to raising (10 ms)
        pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
        shm.data->exit[i].gate.status = 'R';
        pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);
        usleep(10000);

        // Set gate to open
        pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
        shm.data->exit[i].gate.status = 'O';
        pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);
        pthread_cond_signal(&shm.data->exit[i].gate.gatecond);

        // Wait for vehicle to automatically close gate
        usleep(20000);
        
        // Set gate to lowering (10ms)
        pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
        shm.data->exit[i].gate.status = 'L';
        pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);
        usleep(10000);

        // Set gate to closed
        pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
        shm.data->exit[i].gate.status = 'C';
        pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);
    }   
    // FIRE -> set all gates to open
    fireSignal();
    return 0;
}

void generateBill(char* numPlate) {
    // Calculate bill
    FILE *billingFile;
    clock_t entranceTimeCar = vehicleStorage.vehicle[findIndex(&vehicleStorage,numPlate)].entranceTime;
    bill = (double)(clock() - entranceTimeCar)/ CLOCKS_PER_SEC * 50; // FOR NOW
    moneyEarned += bill;
    printf("B - %s bill $%.2f\n", numPlate, bill);

    // Create/append to file
    if ((billingFile = fopen("billing.txt", "r")))
    {
        billingFile = fopen("billing.txt", "a");
        fprintf(billingFile, "%s  -  $%.2f\n", numPlate, bill);
        fclose(billingFile);
    }
    else
    {
        printf("B - file doesn't exist - creating billing.txt file...\n");
        billingFile = fopen("billing.txt", "w");
        fprintf(billingFile, "%s  -  $%.2f\n", numPlate, bill);
        fclose(billingFile);
    }
}


// Generates random numbers in range [lower, upper]. 
// https://www.geeksforgeeks.org/generating-random-number-range-c/?fbclid=IwAR1a4I7mqxidG7EHit34MRmTLgge9xMfBQtw8TcCXVlYC9_QqrATtfESm94
double generateRandom(int lower, int upper)
{    
    double num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

void createThreads(void){
    // Attribute initialise
    pthread_mutexattr_t attr_m;
    pthread_mutexattr_init(&attr_m);
    pthread_mutexattr_setpshared(&attr_m, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&vehicleStorageMutex, &attr_m);
    storageInit(&vehicleStorage);

    pthread_t entranceLPR_thread[ENTRANCES];
    pthread_t levelLPR_thread[LEVELS];
    pthread_t levelController_thread[LEVELS];
    pthread_t exitLPR_thread[EXITS];
    pthread_t exitController_thread[EXITS];
    pthread_t checkTimes_thread[LEVELS];
    pthread_t exitBoomgate_thread[EXITS];
    pthread_t entranceBoomgate_thread[ENTRANCES];
    pthread_t statusDisp_thread;
 
    pthread_create(&statusDisp_thread, NULL, &statusDisplay, NULL);
    int i;
    for (i = 0; i < ENTRANCES; i++){
        int* a = malloc(sizeof(int));
        *a = i;
        pthread_create(&entranceLPR_thread[i], NULL, &entranceLPR, a);   
        pthread_create(&entranceBoomgate_thread[i],NULL, &entranceBoomgate, a);
    }

    for (i = 0; i < EXITS; i++){
        int* b = malloc(sizeof(int));
        *b = i;        
        plateInit(&exitQueue[i]); 
        pthread_mutex_init(&exitQueueMutex[i], NULL);
        pthread_create(&exitLPR_thread[i], NULL, &exitLPR, b);
        pthread_create(&exitController_thread[i], NULL, &exitController, b);
        pthread_create(&exitBoomgate_thread[i], NULL, &exitBoomgate, b);
    }

    for (i = 0; i < LEVELS; i++){
        int* c = malloc(sizeof(int));
        *c = i;        
        plateInit(&levelQueue[i]);
        pthread_mutex_init(&levelQueueMutex[i], NULL);
        pthread_mutex_init(&levelCapacityMutex[i], NULL);
        pthread_create(&checkTimes_thread[i], NULL, &checkTimes, c);
        pthread_create(&levelLPR_thread[i], NULL, &levelLPR, c);
        pthread_create(&levelController_thread[i], NULL, &levelController, c);
    }

    pthread_join(statusDisp_thread, NULL);
    for (i = 0; i < ENTRANCES; i++){     
        pthread_join(entranceLPR_thread[i], NULL);
        pthread_join(entranceBoomgate_thread[i], NULL);
    }

    for (i = 0; i < EXITS; i++){
        pthread_join(exitLPR_thread[i], NULL);
        pthread_join(exitController_thread[i], NULL);
        pthread_join(exitBoomgate_thread[i], NULL);
    }

    for (i = 0; i < LEVELS; i++){
        pthread_join(checkTimes_thread[i], NULL);
        pthread_join(levelLPR_thread[i], NULL);
        pthread_join(levelController_thread[i], NULL);
    }
}

// Reads contents of supplied file 
void readFile(char *filename){
    FILE* file = fopen(filename, "r");
    // Fill array
    int i = 0;
    while (fgets(allowedPlates[i], 10, file)) {
        allowedPlates[i][strlen(allowedPlates[i]) - 1] = '\0';
        i++;
    }
}

void *statusDisplay(void *args) {
    system("clear");
    // Clear screen to prevent scrolling
    char entrancePlateLPR[8];
    char entranceGateStatus;
    char entranceInfoSign;
    char exitPlateLPR[8];
    char exitGateStatus;

    for (;;) {
        // Display current state of each entrance LPR, boomgate and info sign
        // Grab entrance LPR plate
        for (int i =0; i < ENTRANCES; i++) {
            pthread_mutex_lock(&shm.data->entrance[i].LPRSensor.LPRmutex);
            strcpy(entrancePlateLPR, shm.data->entrance[i].LPRSensor.plate);
            pthread_mutex_unlock(&shm.data->entrance[i].LPRSensor.LPRmutex);
            // Grab entrance LPR gate status
            pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
            entranceGateStatus = shm.data->entrance[i].gate.status;
            pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);
            // Grab entrance LPR gate status
            pthread_mutex_lock(&shm.data->entrance[i].informationSign.ISmutex);
            entranceInfoSign = shm.data->entrance[i].informationSign.display;
            pthread_mutex_unlock(&shm.data->entrance[i].informationSign.ISmutex);

            // Print out the read values
            printf("----------------------------Entrance %d information----------------------------\n", i+1);
            printf("LPR plate: %s \n", entrancePlateLPR);
            printf("Boom gate status: %c\n", entranceGateStatus);
            printf("Information display: %c\n", entranceInfoSign); 
        }
        // Display current state of each exit LPR, boomgate
        // Grab the exit LPR plate
        for (int i =0; i < EXITS; i++) {
            pthread_mutex_lock(&shm.data->exit[i].LPRSensor.LPRmutex);
            strcpy(exitPlateLPR, shm.data->exit[i].LPRSensor.plate);
            pthread_mutex_unlock(&shm.data->exit[i].LPRSensor.LPRmutex);
            // Grab the exit LPR gate status
            pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
            exitGateStatus = shm.data->exit[i].gate.status;
            pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);

            printf("------------------------------Exit %d information------------------------------\n", i+1);
            printf("LPR plate: %s\n", exitPlateLPR);
            printf("Boom gate status: %c\n", exitGateStatus);
        }
        // Display current state of each temperature sensor 
        // Grab temp sensor data
        for (int i =0; i < LEVELS; i++) {
            printf("-----------------------------Level %d information------------------------------\n", i+1);
            printf("Temperature Sensor status: %s\n", (shm.data->level[i].fireAlarm == '0') ? "No fire detected..." : "FIRE DETECTED!");
            // Display Number of vehicles  and  maximum  capacity  on  each  level
            printf("Currently %d vehicles on level %d, which has a maximum capacity of %d\n", levelCapacity[i], i+1, CAPACITY);
        }
        printf("------------------------------------------------------------\n");         
        printf("Total billing recorded by the manager thus far: $%.2f\n", moneyEarned);//, totalRevenue);
        //printf("------------------------------------------------------------\n");

        usleep(50000);
        system("clear");
    }
}

void evactuationSequence(void){
    for (int i = 0; i < ENTRANCES; i++){
        pthread_mutex_lock(&shm.data->entrance[i].informationSign.ISmutex);
        shm.data->entrance[i].informationSign.display = 'E';
        pthread_mutex_unlock(&shm.data->entrance[i].informationSign.ISmutex);
    }
    usleep(20000);
    for (int i = 0; i < ENTRANCES; i++){
        pthread_mutex_lock(&shm.data->entrance[i].informationSign.ISmutex);
        shm.data->entrance[i].informationSign.display = 'V';
        pthread_mutex_unlock(&shm.data->entrance[i].informationSign.ISmutex);
    }
    usleep(20000);
    for (int i = 0; i < ENTRANCES; i++){
        pthread_mutex_lock(&shm.data->entrance[i].informationSign.ISmutex);
        shm.data->entrance[i].informationSign.display = 'A';
        pthread_mutex_unlock(&shm.data->entrance[i].informationSign.ISmutex);
    }
    usleep(20000);
    for (int i = 0; i < ENTRANCES; i++){
        pthread_mutex_lock(&shm.data->entrance[i].informationSign.ISmutex);
        shm.data->entrance[i].informationSign.display = 'C';
        pthread_mutex_unlock(&shm.data->entrance[i].informationSign.ISmutex);
    }
    usleep(20000);
    for (int i = 0; i < ENTRANCES; i++){
        pthread_mutex_lock(&shm.data->entrance[i].informationSign.ISmutex);
        shm.data->entrance[i].informationSign.display = 'U';
        pthread_mutex_unlock(&shm.data->entrance[i].informationSign.ISmutex);
    }
    usleep(20000);
    for (int i = 0; i < ENTRANCES; i++){
        pthread_mutex_lock(&shm.data->entrance[i].informationSign.ISmutex);
        shm.data->entrance[i].informationSign.display = 'A';
        pthread_mutex_unlock(&shm.data->entrance[i].informationSign.ISmutex);
    }
    usleep(20000);
    for (int i = 0; i < ENTRANCES; i++){
        pthread_mutex_lock(&shm.data->entrance[i].informationSign.ISmutex);
        shm.data->entrance[i].informationSign.display = 'T';
        pthread_mutex_unlock(&shm.data->entrance[i].informationSign.ISmutex);
    }
    usleep(20000);
    for (int i = 0; i < ENTRANCES; i++){
        pthread_mutex_lock(&shm.data->entrance[i].informationSign.ISmutex);
        shm.data->entrance[i].informationSign.display = 'E';
        pthread_mutex_unlock(&shm.data->entrance[i].informationSign.ISmutex);
    }
    usleep(20000);
}

void fireSignal(void){
    // FIRE -> set all gates to open
    for (int i = 0; i < ENTRANCES; i++){
        pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
        shm.data->entrance[i].gate.status = 'R';
        pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);
        usleep(10000);
        pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
        shm.data->entrance[i].gate.status = 'O';
        pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);
    }
    for (int i = 0; i < EXITS; i++){
        pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
        shm.data->exit[i].gate.status = 'R';
        pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);
        usleep(10000);
        pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
        shm.data->exit[i].gate.status = 'O';
        pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);
    }
    for (int i = 0; i < LEVELS; i++){
        pthread_mutex_lock(&levelCapacityMutex[i]);
        levelCapacity[i] = 0;
        pthread_mutex_unlock(&levelCapacityMutex[i]);
    }
    // FIRE -> show evacuation sequence
    while (fire){
        evactuationSequence();
    }
}

void checkFire(void){
    // Check for fire!
    for (int i = 0; i < LEVELS; i++){
        if (shm.data->level[i].fireAlarm == '1'){
            fire = 1;
        }
    }
}