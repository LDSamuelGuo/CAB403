#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "sharedmemory.c"
/*****************Method******************************/
bool get_shared_object_T(share_memory_t* shm, const char* share_name )
{     

     //remove memory
     shm_unlink(share_name);

    // Assign share name to shm->name.
    shm->name = share_name;
    (shm->fd = shm_open(share_name, O_CREAT | O_RDWR, 0666);
    //check if shm_open worked
    if (shm->fd == -1){
        shm->data = NULL;
        return false;
    }

    // Set the capacity of the shared memory object via ftruncate.
    if (ftruncate(shm->fd,sizeof(shared_data_t)) == -1){
        shm->data = NULL;
        return false;
    }

    // Map memory segment
    shm->data = mmap(0, sizeof(shared_data_t), PROT_WRITE, MAP_SHARED, shm->fd, 0);
    if (shm->data == (void *)-1)
    {
        return false;
    }

    //Modify the remaining stub only if necessary
    return true;
}

void destroy_shared_object( shared_memory_t* shm ) {
    // Remove the shared memory object.
    munmap(shm, 48);
    shm->data = NULL;
    shm->fd = -1;
    shm_unlink(shm->name);
}


bool get_shared_object_TW(share_memory_t* shm, const char* share_name )
{         

    // Assign share name to shm->name.
    shm->name = share_name;
    shm->fd = shm_open(share_name, O_RDWR, 0)
   if (shm->fd  < 0)
   {
        shm->data = NULL;
        return false;
    }

    // Map memory segment
    shm->data = mmap(0, sizeof(shared_data_t), PROT_WRITE, MAP_SHARED, shm->fd, 0);
    if (shm->data == (void *)-1)
    {
        return false;
    }

    //Modify the remaining stub only if necessary
    return true;
}




// Set Up Mutex/Condition Variables By Default
void setDefaultValues(shared_memory_t shm){
    pthread_mutexattr_t attr_m;
    pthread_condattr_t attr_c;
    pthread_mutexattr_init(&attr_m);
    pthread_condattr_init(&attr_c);
    pthread_mutexattr_setpshared(&attr_m, PTHREAD_PROCESS_SHARED);
    pthread_condattr_setpshared(&attr_c, PTHREAD_PROCESS_SHARED);
//Entries
    for (int i = 0;i < ENTRANCES;i++){
         //  Information signs
        pthread_mutex_init(&shm.data->entrance[i].informationSign.ISmutex, &attr_m);
        pthread_cond_init(&shm.data->entrance[i].informationSign.IScond, &attr_c);
        
       //  Boom Gates
        pthread_mutex_init(&shm.data->entrance[i].gate.gatemutex, &attr_m);
        pthread_cond_init(&shm.data->entrance[i].gate.gatecond, &attr_c);
          shm.data->entrance[i].gate.status = 'C';
        strcpy(shm.data->entrance[i].LPRSensor.plate, "xxxxxx");
          //  LPR sensors
        pthread_mutex_init(&shm.data->entrance[i].LPRSensor.LPRmutex, &attr_m);
        pthread_cond_init(&shm.data->entrance[i].LPRSensor.LPRcond, &attr_c);
       

        
    }
//Exits
    for (int i = 0;i < EXITS;i++){
        
        // Boom Gates
        pthread_mutex_init(&shm.data->exit[i].gate.gatemutex, &attr_m);
        pthread_cond_init(&shm.data->exit[i].gate.gatecond, &attr_c);
         shm.data->exit[i].gate.status = 'C';
        strcpy(shm.data->exit[i].LPRSensor.plate, "xxxxxx");
         //  LPR sensors
        pthread_mutex_init(&shm.data->exit[i].LPRSensor.LPRmutex, &attr_m);
        pthread_cond_init(&shm.data->exit[i].LPRSensor.LPRcond, &attr_c);

        
    }
//Levels
    for (int i = 0;i < LEVELS;i++){
        //  LPR sensors
        pthread_mutex_init(&shm.data->level[i].LPRSensor.LPRmutex, &attr_m);
        pthread_cond_init(&shm.data->level[i].LPRSensor.LPRcond, &attr_c);
        shm.data->level[i].fireAlarm = '0';
        shm.data->level[i].tempSensor = 24;

        // Initiliase number plate to be xxxxxx
        strcpy(shm.data->level[i].LPRSensor.plate, "xxxxxx");
    }
printf("All mutexes created.\n")
}
