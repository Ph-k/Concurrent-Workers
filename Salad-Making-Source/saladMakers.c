#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include "MySharedMemLib.h"
#include "logFileWriter.h"

/*This function takes two integers (range of time) and returns a radrom value between them.
Used to get a radrom time for the salad maker, given the t1,t2 arguments*/
int radromIntergerIn(int low, int high){
	static int t=0; 
	if(t==0){ srand(time(0)); t++;} //Srand is initialized only one time, on the first execution of the function

	return (rand() % (high - low + 1)) + low;
}

int main(int argc, char **argv){

	if(argc!=7) {printf("saladmaker: check arguments format!\n"); return -3;}//Salad maker needs 3 arguments

	int lb=-1,ub=-1,shmid=-1,i;

	for(i=1; i<argc; i++){//Reading all the given arguments
		if( strcmp(argv[i],"-t1") == 0 ){
			lb = atoi(argv[++i]);
		}else if( strcmp(argv[i],"-t2") == 0){
			ub = atoi(argv[++i]);
		}else if( strcmp(argv[i],"-s") == 0){
			shmid = atoi(argv[++i]);
		}
	}

	if(lb<0 || ub<0 || shmid<0 || lb>ub){//Error message if any type errors where made to the comand line arguments
		printf("saladmaker: one or more of '-t1' '-t2' '-s' arguments where mistyped or not given at all! (check readme for format)\n");
		return -3;
	}

	//Attaching the shared memory segment associated with the the given shared memory id
	Kitchen *kitchen = (Kitchen*) attachSharedMem(shmid);
	if(kitchen==(void*)-1) {printf("Invalid shared memory id (check -s argument)\n"); return -1;}
	sem_t *materials_sem;

	int makerType,numofSlds;

	//Here the salad makers read and modify variables in the shared memory, thus this part of the code is CS and is protected with a semaphore
	sem_wait(&(kitchen->smAccess));

	//Based on the curMaker variable, processes learn which salad maker will implement
	materials_sem = &(kitchen->saladMaker[kitchen->curMaker-1]); //Each salad maker has a semaphore, through which he demands and takes materials
	makerType = kitchen->curMaker;//This process is the salad maker number curMaker
	printf("I am a salad maker number %d\n",kitchen->curMaker);
	if(kitchen->curMaker<3){ //If more salad makers are to be created
		kitchen->curMaker += 1; //The next salad maker who will use the curMaker variable to learn which salad maker he is, will be the next form this one 
	}else{
		kitchen->curMaker = 0; //Otherwise there are no more salad makers to arrive, and the curMaker variable is set to 0
	}

	sem_post(&(kitchen->smAccess));//End of changes in the shared memory shegment/end of CS, change to the semaphore

	sem_post(materials_sem); //Chef is informed through this semaphore that this salad maker has arrived

	//Iniliazing the log file of this procces
	char log[12];
	sprintf(log,"SaladMaker%d",makerType);
	initializeLog(log);

	sem_wait(&(kitchen->smAccess));
	numofSlds = kitchen->numofSlds; //Seeing how many salads are to be made, from shared memory
	sem_post(&(kitchen->smAccess));
	//if(numofSlds>=makerType)
	while(numofSlds>0){//While there are salads to be made

		//Salad maker waits for the ingredients he is missing
		writeToLog(log,"Waiting for ingredients");
		sem_wait(materials_sem);//The whole waiting and taking ingredients process, is represented through this semaphore

		sem_post(&(kitchen->chefNotify));//Maker notifying the chef that he has received the ingredients he demanded
		writeToLog(log,"Get ingredients");

		writeToLog(log,"Start making salad");//The maker starts making the salad now that he has the ingredients
		sleep(radromIntergerIn(lb,ub));//This procedure takes a random amount of time, which range is based on the lb ub arguments

		sem_wait(&(kitchen->smAccess));//The maker requests acces to the shares memory shegment...
		kitchen->numofSlds -= 1;//..In order to decrease the number of remaining salads, since he just made one. 
		//(this part of the code represents the act of giving the salad to a waiter)
		numofSlds = kitchen->numofSlds;
		sem_post(&(kitchen->smAccess));

		printf("Just made a salad\n");
		writeToLog(log,"End making salad");
	}

	printf("Done!\n");

	sem_wait(&(kitchen->smAccess));//Salad maker finished it's job, this fact is captured in the curMaker variable...
	//...the sum of 3 primes numbers is used, so that the chef can understand which combination of makers have finished
	switch(makerType){
		case 1:
			kitchen->curMaker += 3; //Maker 1 prime value is 3
			break;
		case 2:
			kitchen->curMaker += 5; //Maker 2 value is 5
			break;
		case 3:
			kitchen->curMaker += 7; //Maker 3 value is 5
			break;
	}
	sem_post(&(kitchen->smAccess));

	/*A post to the chef semaphore is needed, 
	because due to the concurrency of the problem, the chef might went to give ingredients to this particialar salad maker
	before it was "anounced" that all the needed salads where made*/
	sem_post(&(kitchen->chefNotify));

	closeSharedMem(kitchen);//Closing the share momory, on what regards this process
}