#pragma once

/* Code from https://github.com/Ph-k/Concurrent-Workers. Philippos Koumparos (github.com/Ph-k)*/

#include <semaphore.h>
#include <time.h>

//This structure represents the variables that exist in the shared memory segment
struct sharedMem{
	sem_t saladMaker[3]; //One semaphore for each salad maker, which also represent the action of demanding and taking ingredients
	sem_t chefNotify; //One semaphore with which the salad makers will inform the chef when they recieve the ingredients
	sem_t smAccess; //On semaphore which evryone will use when they want to read/modify the following variables
	int numofSlds; //Total number of salad remaining
	//When used on the start of the programs it represents which salad makers have arrived...
	int curMaker; //... when used on the end of the programs it represents which salad makers have finished their work
};

typedef struct sharedMem Kitchen;

int createSharedMem(size_t size); //This function, creates/allocates the shared memory segment with my "default" options
void* attachSharedMem(int id); //This function attaches a shared memory segment using C's library functions
int closeSharedMem(void *m); //This function detaches a shared memory segment using C's library functions
int removeSharedMem(int id); //This functions removes/deletes a shared memory segment