/*HEADER*/ 
#include <assert.h>
#include "sharedmemorymethod.c"

#define SHARE_NAME "PARKING"
#define ADDEDTEMP 8
#define TEMPCHANGE_WINDOW 30
#define LOOPBOUNDARIE 1e9


// FUNCTION DECLERATIONS  // 
//C99 standardized on int16_t//
int16_t niceDta(int16_t arr[TEMPCHANGE_WINDOW]);
int16_t medianDiscovery(int16_t array[TEMPCHANGE_WINDOW] , int n);
void Fixedarray(int16_t array[TEMPCHANGE_WINDOW] , int n);
int SortedTemperature(int16_t arr[LEVELS][TEMPCHANGE_WINDOW], int index);
int rateOfIncrease(int16_t arr[LEVELS][TEMPCHANGE_WINDOW], int index);
void loopBoundary(int i);


/*PUBLIC VARIABLES */
int alarm_active = 0;
int16_t rawData[LEVELS][TEMPCHANGE_WINDOW] = {0};
int16_t nData[LEVELS][TEMPCHANGE_WINDOW] = {0};
shared_memory_t shm;

// --------------------------------------------- MAIN --------------------------------------------- // 
int main()
{ 
    // Initialise Shared memory
    create_shared_object_R(&shm, SHARE_NAME);
    // Declare local variables
    int j = 0, levelOfFire = 0, lim;
    for(lim=0; lim < LOOPBOUNDARIE; ++j ) {
        // Loop through levels checking temperatures
        int i=0;
        while (i < LEVELS)
        {
            // Read temperature data from temperature sensor
            rawData[i][j % TEMPCHANGE_WINDOW] = shm.data->level[i].tempSensor;
            printf("Temperature sensor reading from level %d = %d\n", i+1, rawData[i][j% TEMPCHANGE_WINDOW]);
            if (j > 4) {
                // Find 5 most recent temperature readings
                int index = j % TEMPCHANGE_WINDOW, z = 0, numSamples = 5;
                int16_t mostRecentReadings[5];
                for (int currentIndex = index - numSamples; currentIndex < index; currentIndex++) {
                    mostRecentReadings[z] = rawData[i][currentIndex];
                    z++;
                }
                // Calculate the recent smoothed data
                int16_t medianReading = niceDta(mostRecentReadings);
                nData[i][j % TEMPCHANGE_WINDOW] = medianReading;
                // Check temp readings on every floor for both methods of detection
                if (SortedTemperature(nData, i) || rateOfIncrease(nData, i)) {
                    levelOfFire = i;
                    break;
                }
            }
            usleep(2000); 
            ++i;   
        }
        loopBoundary(lim);

        // Set stauts of fire alarm to '1' in shared memory when activated
        if (alarm_active == 1) {
            shm.data->level[levelOfFire].fireAlarm = '1';
            break;
        }
    }
}

//HELPER FUNCTIONS // 

//Tests if 90% of readings 58 degrees or over
int SortedTemperature(int16_t arr[LEVELS][TEMPCHANGE_WINDOW], int index)
{
    // Check alarm is not already active
    assert(alarm_active == 0); 
    // Count recent temperature readings over 58 degrees
	int cnt = 0, j=0;
    while ( j < TEMPCHANGE_WINDOW)
    {
        if (arr[index][j] >= 58)
        {
            cnt++;
            // Check if 90 percent of the readings have exceeded maximum temp
            if (cnt >= TEMPCHANGE_WINDOW * 0.9)
            {
                // Set off alarm
                alarm_active = 1;
                return alarm_active;
            }	
        }
        ++j;
    }
    return 0;
}	

int rateOfIncrease(int16_t arr[LEVELS][TEMPCHANGE_WINDOW], int index){
    // Check alarm is not already active
    assert(alarm_active == 0);
    // Check if the most recent temperature is 8°C (or more) hotter than the 30th most recent temperature 
    if (arr[index][0] != 0 && arr[index][TEMPCHANGE_WINDOW-1] - arr[index][0] >= ADDEDTEMP) {
        alarm_active = 1;
        return alarm_active;
    }
    return 0;
}

int16_t niceDta(int16_t arr[5])
{
    // Require 5 most recent temperatures as per the spec
    int numSamples = 5;
    // Sort the array in ascending order
    Fixedarray(arr, numSamples);
    // Pass the sorted array to calculate the median of array.
    int16_t median = medianDiscovery(arr , numSamples);    
    return median;
}

 // Array and median function
void Fixedarray(int16_t array[TEMPCHANGE_WINDOW] , int n)
{ 
    // Declare local variables
    int16_t i=0 , j=0 , temp=0;

    while (i<n )
    {
        while(j<n-1 )
        {
            if(array[j]>array[j+1])
            {
                temp        = array[j];
                array[j]    = array[j+1];
                array[j+1]  = temp;
            }
            ++j;
        }
        ++i;
    }
}


// function to calculate the median of the array
int16_t medianDiscovery(int16_t array[TEMPCHANGE_WINDOW] , int n)
{
    int16_t median = 0;
    
    // if number of elements are even
    if(n%2 == 0)
        median = (array[(n-1)/2] + array[n/2])/2.0;
    // if number of elements are odd
    else
        median = array[n/2];
    
    return median;
}

//checks if the upper bound of a loop has been reached
//terminates program with error message if true
void loopBoundary(int i)
{
	if (i >= LOOPBOUNDARIE)
	{
		printf("ERROR: Loop Limit Upper Bound Exceeded, Program Will Close");
		exit(1);
	}
}