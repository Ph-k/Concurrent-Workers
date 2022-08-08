#include <stdio.h>
#include <sys/shm.h>
#include "MySharedMemLib.h"

int createSharedMem(size_t size){
	int id = shmget(IPC_PRIVATE,size,0666); //The shared memory segment is created using shmget() and some default options
	if (id == -1)  perror ("Creation of shared memory\n"); //Error message in case of error
	return id;
}

/*The implementations of the rest of the functions don't do something special,
but they are included to complete this "dummy library"*/

void* attachSharedMem(int id){
	return shmat(id,NULL,0);
}

int closeSharedMem(void *m){
	if (shmdt(m) == -1){
		perror ("Detachment of shared memory\n");
		return -1;
	}
	return 0;
}

int removeSharedMem(int id){
	if (shmctl(id, IPC_RMID, 0) == -1){
		perror ("Removal of shared memory\n");
		return -1;
	}
	return 0;
}