#include <semaphore.h>

#define ENTRANCES 5
#define EXITS 5
#define LEVELS 5


/**
 * Information sign 
 */
typedef struct informationSign_t {
    pthread_mutex_t ISmutex;
	pthread_cond_t IScond;
	char display;
} informationSign_t;
/*
boom gate state
*/ 
typedef enum gate_status_t
{
    C,
    O,
    R,
    L
}gate_status_t;

/**
 * Boom gates 
 */
typedef struct gate_t {
	pthread_mutex_t gatemutex;
	pthread_cond_t gatecond;
	gate_status_t status;    
} gate_t;
 
/**
 * LPR sensors
 */
typedef struct LPRSensor_t {
    pthread_mutex_t LPRmutex;
    pthread_cond_t LPRcond;
    char plate[7];
} LPRSensor_t;

/**
 * The different levels
 */
typedef struct level_t {
    LPRSensor_t LPRSensor;
    volatile int16_t tempSensor; 
    volatile char fireAlarm;
} level_t;

/**
 * The different exits
 */
typedef struct exit_t {
    LPRSensor_t LPRSensor;
    gate_t gate;
} exit_t;

/**
 * The different entrances
 */
typedef struct entrance_t {
    LPRSensor_t LPRSensor;
    gate_t gate;
    informationSign_t informationSign;
} entrance_t;

/**
 * Our shared data block.
 */
typedef struct shared_data_t {
    entrance_t entrance[ENTRANCES];

    exit_t exit[EXITS];

    level_t level[LEVELS];
} shared_data_t;

/**
 * A shared memory control structure.
 */
typedef struct shared_memory_t {
    /// The name of the shared memory object.
    const char* name;

    /// The file descriptor used to manage the shared memory object.
    int fd;
    size_t size;
    /// Address of the shared data block. 
    shared_data_t* data;
} shared_memory_t;
