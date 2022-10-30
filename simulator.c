//  HEADER  //
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
#include <ctype.h>
#include <stdint.h>

#include "Parking.c"
#include "sharedmemorymethod.c"

#define SHARE_NAME "PARKING"
#define RANDOM_CHANCE 80
#define CAR_LIMIT 20
#define FIRE 0


// FUNCTION DECLERATIONS // 
int generateRandom(int lower, int upper);
void readFile(char *filename);
void printFile();
char *generatePlate(int probability);
char *randomPlate();
void setDefaultValues(shared_memory_t shm);
void *spawnVehicle(void *args);
void *entranceSimulate(void *arg);
void *tempSensorSimulate(void *arg);

//  PUBLIC VARIABLES // 
char allowedPlates[100][7];
shared_memory_t shm;
vehicleQueue_t entranceQueue[ENTRANCES];
pthread_mutex_t entranceQueueMutex[ENTRANCES];
int selector;

//THREADS // 
void *spawnVehicle(void *args) {
    char* plate;  
    int waitTime, i=0;    
    while (i < CAR_LIMIT){
        // Generate numberplate
        plate = generatePlate(RANDOM_CHANCE);
        selector++;

        // Generate vehicle
       usleep(waitTime);
        waitTime = generateRandom(1,100) * 1000;
        

        // Generate a random entrance 
        int entranceVehicle = generateRandom(0,ENTRANCES - 1);

        printf("The plate %s is arriving at entrance %d\n",plate,entranceVehicle + 1);
        // SPAWN CAR THREAD 
        pthread_mutex_unlock(&entranceQueueMutex[entranceVehicle]);
         addPlate(&entranceQueue[entranceVehicle], plate);
        pthread_mutex_lock(&entranceQueueMutex[entranceVehicle]);
       
        
        i++;
    }
      while ( i < ENTRANCES){
        plateInit(&entranceQueue[i]);
        i++;
    }
    return 0;
} 

void *entranceSimulate(void *arg) {
    int i = *(int*) arg;
    for (;;){
        // Wait for manager LPR thread to signal that LPR is free
        pthread_mutex_lock(&shm.data->entrance[i].LPRSensor.LPRmutex);
        while (strcmp(shm.data->entrance[i].LPRSensor.plate, "000000")){ 
            pthread_cond_wait(&shm.data->entrance[i].LPRSensor.LPRcond, &shm.data->entrance[i].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->entrance[i].LPRSensor.LPRmutex);
        // LRP has been cleared 
        
        // Make sure plate is in queue
        while(entranceQueue[i].size <= 0);

        // Copy a plate into memory
        pthread_mutex_lock(&shm.data->entrance[i].LPRSensor.LPRmutex);
        pthread_mutex_lock(&entranceQueueMutex[i]);
        strcpy(shm.data->entrance[i].LPRSensor.plate, entranceQueue[i].plateQueue[0]);
        pthread_mutex_unlock(&entranceQueueMutex[i]);
        pthread_mutex_unlock(&shm.data->entrance[i].LPRSensor.LPRmutex);

        // Wait 2ms before triggering LPR
        usleep(2000);

        // Signal manager thread with new plate
        pthread_cond_signal(&shm.data->entrance[i].LPRSensor.LPRcond);
        pthread_mutex_lock(&entranceQueueMutex[i]);
        deletePlate(&entranceQueue[i]);
        pthread_mutex_unlock(&entranceQueueMutex[i]);
    }
}
   


//HELPER FUNCTIONS // 

int generateRandom(int lower, int upper)
{    
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

// Reads contents of supplied file 
void readFile(char *filename){
    FILE* file = fopen(filename, "r");

    // Fill array
    
    for(int i = 0; fgets(allowedPlates[i], 10, file); i++) {
        allowedPlates[i][strlen(allowedPlates[i]) - 1] = '\0';
        
    }
    
}





// Constructs a random plate
char* randomPlate(){
    int first = generateRandom(0, 9);
    int second = generateRandom(0, 9);
    char third = generateRandom(0, 9);

    char randomletter1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[generateRandom(0, 25)];
    char randomletter2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[generateRandom(0, 25)];
    char randomletter3 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[generateRandom(0, 25)];

    char *finstr = NULL;
    finstr = malloc(10);
    sprintf(&finstr[0], "%d", first);
    sprintf(&finstr[1], "%d", second);
    sprintf(&finstr[2], "%d", third);    
    finstr[3] = randomletter1;
    finstr[4] = randomletter2;
    finstr[5] = randomletter3;
    finstr[6] = '\0';

    return finstr;
}
// Generates a plate (Random/Allowed)
char* generatePlate(int probability){
    int random = generateRandom(0, 100);
    if (random <= probability){
        return allowedPlates[selector];
    }
    else{
        char *p = randomPlate();
        return p;
    }
}

// Simulates temperatures
void *tempSensorSimulate(void *arg) {
    int i = *(int*) arg;
    int16_t temperature;
    int16_t currentTemp;

    for (;;) {
        usleep(2000);
        if (FIRE == 1) { 
            temperature = (int16_t) generateRandom(58, 65);
            shm.data->level[i].tempSensor = temperature;
        }
        else if (FIRE == 2) { 
            if (shm.data->level[i].tempSensor > 58){
                currentTemp = 24;
            }
            else {
                currentTemp = shm.data->level[i].tempSensor;
            }
            temperature = generateRandom(currentTemp, currentTemp + 2);
            shm.data->level[i].tempSensor = temperature;
        }
        else {
           
            temperature = (int16_t) 24;
            shm.data->level[i].tempSensor = temperature;
        }    
    }
}


// Prints contents of numberplate file (USED FOR TESTING)
void printFile(){
	printf("\n The content of the file  are : \n"); 
    int i= 0;   
    while(i < 100)
    {
        printf("%s, ", allowedPlates[i]);
        i++;
    }
    printf("\n");
}

//main  // 
int main()
{
    selector = 0;
    pthread_t vehicleSpawner;
    pthread_t entranceThread[ENTRANCES];
    pthread_t tempSensorSimulate_thread[LEVELS];
    int i=0;
    while (i < ENTRANCES){
        pthread_mutex_init(&entranceQueueMutex[ENTRANCES], NULL);
        i++;
    }    
    // Initialise random seed
    time_t t;
    srand((unsigned) time(&t));
    // Create shared memory object
    get_shared_object_T(&shm, SHARE_NAME);
    setDefaultValues(shm);
   
    readFile("plates.txt");   


    pthread_create(&vehicleSpawner, NULL, &spawnVehicle, NULL);
    
   while( i < LEVELS){
        int* z = malloc(sizeof(int));
        *z = i;
        pthread_create(&tempSensorSimulate_thread[i], NULL, &tempSensorSimulate, z);
        i++;
    }
   
    while ( i < ENTRANCES){
        int* p = malloc(sizeof(int));
        *p = i;
        pthread_create(&entranceThread[i], NULL, &entranceSimulate, p);
        i++;
    }
    // Join threads 
    pthread_join(vehicleSpawner,NULL);
      while (i < LEVELS){
        pthread_join(tempSensorSimulate_thread[i],NULL);
        i++;
    }
     while (i < ENTRANCES){
        pthread_join(entranceThread[i], NULL);
        i++;
    }
 
}