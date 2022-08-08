/* Code from https://github.com/Ph-k/Concurrent-Workers. Philippos Koumparos (github.com/Ph-k)*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include "MySharedMemLib.h"
#include "logFileWriter.h"
#include "list.h"

int printCommonIntervals();

int main(int argc, char *argv[]){

	if(argc!=5) {printf("chef: check arguments format!\n"); return -3;}//Chef needs 2 arguments:

	int numofSlds=-1,mantime=-1,i,makersSalads[3]={0,0,0};

	for(i=1; i<argc; i++){//Reading all the given arguments
		if( strcmp(argv[i],"-n") == 0 ){
			numofSlds = atoi(argv[++i]);
		}else if( strcmp(argv[i],"-m") == 0){
			mantime = atoi(argv[++i]);
		}
	}

	if(numofSlds<0 || mantime<0){//Error message if any logical or type errors where made to the comand line arguments
		printf("chef: one or more of '-n' '-m' arguments where mistyped or not given at all! (check readme for format)\n");
		return -3;
	}

	initializeLog("Chef");//Initializing the log file of the chef

	/*Creating shared memory shegment which will include all the needed semaphores and variables,
	 more details on MySharedMemLib.h:6 explained with comments, please read them before proceeding*/
	int shmid = createSharedMem(sizeof(Kitchen));

	//Attaching the newly created shared memory shegment
	Kitchen *kitchen = (Kitchen*) attachSharedMem(shmid);

	//Initializing all the needed semaphores
	sem_init(&(kitchen->smAccess),1,0);
	sem_init(&(kitchen->chefNotify),1,0);

	for(i=0; i<3; i++)
		sem_init(&(kitchen->saladMaker[i]),1,0);
	
	kitchen->curMaker = 1;//The first maker will be saladmaker1
	kitchen->numofSlds = numofSlds;//Initializing the number of salads that need to be made

	//End of initialization of the shared memory shegment, chaching the value of the semaphore so others can access it
	sem_post(&(kitchen->smAccess));

	printf("Shared memory id: %d\n",shmid);//Priting the shared memory id, in order for it to be given to the saladmakers

	int makerCount=0,workingMakers;
	char *logMessage = malloc(sizeof(char)*25);

	for(i=0; i<3; i++)
		sem_wait(&(kitchen->saladMaker[i])); //Chef waits for everyone before he starts working

	while(numofSlds>0){//While there are salads to be made
		//Chosing who will get his ingredients next (there are given cyclically)
		switch(makerCount){//Depending on which ingredients are chosen, the right message is writen in the log
			case 0:
				writeToLog("Chef","Selecting ingredients Tomato Green pepper");
				break;
			case 1:
				writeToLog("Chef","Selecting ingredients Tomato Small onion");
				break;
			case 2:
				writeToLog("Chef","Selecting ingredients Green pepper Small onion");
				break;
		}

		sem_post(&(kitchen->saladMaker[makerCount]));//Giving ingredients and notifying salad maker, through his semaphore
		sprintf(logMessage,"Notify saladmaker %d",makerCount+1);
		writeToLog("Chef",logMessage);//Writing the event in the log file
		sem_wait(&(kitchen->chefNotify));//Waiting for confirmation from the salad maker that he took the ingredients

		writeToLog("Chef","Man time for resting");
		sleep(mantime);//Chef rests for the given amount of time

		//Chef will check for the number of remaining salads, this variable is in the shared shegment and this action is protected using semaphore
		sem_wait(&(kitchen->smAccess));
		numofSlds = kitchen->numofSlds;
		sem_post(&(kitchen->smAccess));

		makerCount = (makerCount+1)%3;//Cyclically choosing the next salad maker
	}

	/*At this stage the kitchen->curMaker value represents which salad makers are still active
	the active salad makers are waiting for ingredients since they have not notice that all the salads are made 
	(that's because each salad maker checks how many salads are left in the end of their loop)
	in order to avoid deadlock, chef posts on their ingredients semaphores, in order for them to re-check how many salads left, and terminate
	more in saladMakers.c:99*/
	
	sem_wait(&(kitchen->smAccess));
	workingMakers = kitchen->curMaker;
	sem_post(&(kitchen->smAccess));
	while(workingMakers<15){//Value 15 explained at line 126
		switch(workingMakers){
			case 3://A vaule of 3 means that only saladmaker1 has finished
				//So the rest are "woken up"
				sem_post(&(kitchen->saladMaker[1]));
				sem_post(&(kitchen->saladMaker[2]));
				break;
			case 5://A vaule of 3 means that only saladmaker2 has finished
				sem_post(&(kitchen->saladMaker[0]));
				sem_post(&(kitchen->saladMaker[2]));
				break;
			case 7://A vaule of 3 means that only saladmaker3 has finished
				sem_post(&(kitchen->saladMaker[0]));
				sem_post(&(kitchen->saladMaker[1]));
				break;
			case 8://A vaule of 8 means that both saladmaker1 and saladmaker2 have finished
				sem_post(&(kitchen->saladMaker[2]));
				break;
			case 10://A vaule of 8 means that both saladmaker1 and saladmaker3 have finished
				sem_post(&(kitchen->saladMaker[1]));
				break;
			case 12://A vaule of 8 means that both saladmaker2 and saladmaker3 have finished
				sem_post(&(kitchen->saladMaker[0]));
				break;
		}
		sem_wait(&(kitchen->smAccess));
		workingMakers = kitchen->curMaker;
		sem_post(&(kitchen->smAccess));
	}//Value 15 means that all the salad makers have finished

	//destroying all the semaphores
	for(i=0; i<3; i++)
		sem_destroy(&(kitchen->saladMaker[i]));

	sem_destroy(&(kitchen->smAccess));
	sem_destroy(&(kitchen->chefNotify));
	removeSharedMem(shmid);//Deleting the shared memory shegment

	//Getting the number of salads each maker made from their log files
	makersSalads[0] = getTotalSaladsFromLog(1);
	makersSalads[1] = getTotalSaladsFromLog(2);
	makersSalads[2] = getTotalSaladsFromLog(3);
	printf("Total #salads: %d\n",makersSalads[0]+makersSalads[1]+makersSalads[2]);
	int *makersPID = getMakersPIDFromLog();//Getting the pids of the salad makers from the log file

	for(i=0; i<3; i++)
		printf("#salads of salad_maker%d [%d] = %d\n",i+1,makersPID[i],makersSalads[i]);

	free(makersPID);
	free(logMessage);
	printCommonIntervals();
	
	return 0;
}


int checkIfIntervalIn(struct makerTime interval1,struct makerTime interval2);//Forward declaration (function at line 282)

int printCommonIntervals(){
	//Initializing lista for the times where salad makers where working...
	list *times1 = initialize(),
		 *times2 = initialize(),
		 *times3 = initialize(),
		 *commonIntervals = initialize();//...and also for the list for the final intervals
	struct makerTime *intervalEnd=NULL;
	//Populating the times lists with times in which the salad makers were working, more on logFileWriter.c/h and list.c/h
	getTimeIntervalsFromLog(times1,1);
	getTimeIntervalsFromLog(times2,2);
	getTimeIntervalsFromLog(times3,3);

	listNode *mainInterval, *intervalT1, *intervalT2, *tInterval, *temp;

	printf("Time intervals: (in increasing order)\n\n");

	int i,j;
	for(i=0; i<3; i++){//Each time intreval is compared to all the intervals of the other salad makers
		switch(i){
			case 0://First we compare salad makers 1 intervals with salad makers 2 and 3
				mainInterval = times1->head;
				intervalT1 = times2->head;
				intervalT2 = times3->head;
				break;
			case 1://Then we compare salad makers 2 intervals with salad makers 1 and 3
				mainInterval = times2->head;
				intervalT1 = times1->head;
				intervalT2 = times3->head;
				break;
			case 2://Finally we compare salad makers 3 intervals with salad makers 2 and 1
				mainInterval = times3->head;
				intervalT1 = times1->head;
				intervalT2 = times2->head;
		}
		for(j=0; j<2; j++){//The mainInterval is compared with the other two intervals 
			switch(j){
				case 0:
					temp = mainInterval; //Saving the start of the list
					break;
				case 1:
					mainInterval = temp; //Resetting the start of the list for the seconde comparison
			}
			while(mainInterval!=NULL){//Traversing the main list
				switch(j){//Chosing the correct comparison list according to the j's loop itteration
					case 0:
						tInterval = intervalT1;
						break;
					case 1:
						tInterval = intervalT2;
				}
				intervalEnd=NULL; //Initializing the common internal as not found
				while(tInterval!=NULL){/*From now on we check intervals, there are two cases:
					1. the whole inverval from star to end is "included" to the main interval
					2. only the end of the interval is" included" to the main interval 
					(the case of an interval starting in the main interval end ending after it, is also covered beucase all intervals are checked)
					*/
					if(checkIfIntervalIn(mainInterval->time,tInterval->time)==0){//Case 1 check
						insertSorted(commonIntervals,tInterval->time);//If the interval must be included in the awnser it is inserted in the awnser list at it's sorted possition
					}else if(compareTime(mainInterval->time.hours, mainInterval->time.mins, mainInterval->time.secs, mainInterval->time.msecs,
						tInterval->time.hours, tInterval->time.mins, tInterval->time.secs, tInterval->time.msecs) >= 0 //The main interval must have started after
						&&
						compareTime(mainInterval->time.endHours, mainInterval->time.endMins, mainInterval->time.endSecs, mainInterval->time.endMsecs,
						tInterval->time.endHours, tInterval->time.endMins, tInterval->time.endSecs, tInterval->time.endMsecs) >= 0 //The main interval must end after
						&&
						compareTime(mainInterval->time.hours, mainInterval->time.mins, mainInterval->time.secs, mainInterval->time.msecs,
						tInterval->time.endHours, tInterval->time.endMins, tInterval->time.endSecs, tInterval->time.endMsecs) <= 0 //And the end must be in the main interval
						){//Case 2 check
						if(intervalEnd==NULL){//If such an interval is found for the first time
							//It starts from the start of the main interval
							intervalEnd = malloc(sizeof(struct makerTime));
							intervalEnd->hours = mainInterval->time.hours;
							intervalEnd->mins = mainInterval->time.mins;
							intervalEnd->secs = mainInterval->time.secs;
							intervalEnd->msecs = mainInterval->time.msecs;

							//And ends at the end of the 'other' interval
							intervalEnd->endHours = tInterval->time.endHours;
							intervalEnd->endMins = tInterval->time.endMins;
							intervalEnd->endSecs = tInterval->time.endSecs;
							intervalEnd->endMsecs = tInterval->time.endMsecs;

						//In case another interval of this kind is found, and it ends after the one we have found, the common interval...
						}else if(compareTime(intervalEnd->endHours, intervalEnd->endMins, intervalEnd->endSecs, intervalEnd->endMsecs,
						         tInterval->time.endHours, tInterval->time.endMins, tInterval->time.endSecs, tInterval->time.endMsecs) < 0){
							
							//Must be extented to the end of the new "other" interval
							intervalEnd->endHours = tInterval->time.endHours;
							intervalEnd->endMins = tInterval->time.endMins;
							intervalEnd->endSecs = tInterval->time.endSecs;
							intervalEnd->endMsecs = tInterval->time.endMsecs;
						}
					}
					tInterval = tInterval->next;
				}
				if(intervalEnd!=NULL){//If a common interval of case 2 was found, it is inserted sorted in the answer
					insertSorted(commonIntervals,*intervalEnd);
					free(intervalEnd);
				}
				mainInterval = mainInterval->next;
			}
		}
		
	}

	mainInterval = commonIntervals->head;
	while(mainInterval!=NULL){//Priting the list values, which contain the sorted answer
		printf("[%2d:%2d:%2d.%2d (start) - %2d:%2d:%2d.%2d (end)]\n",
				mainInterval->time.hours, mainInterval->time.mins, mainInterval->time.secs, mainInterval->time.msecs,
				mainInterval->time.endHours, mainInterval->time.endMins, mainInterval->time.endSecs, mainInterval->time.endMsecs);
		mainInterval = mainInterval->next;
	}

	//Deleting all the lists structures
	destroy(times1);
	destroy(times2);
	destroy(times3);
	destroy(commonIntervals);
	return 0;
}

/*Check if the whole interval2 is included in interval1 by comparing their start-end time values
value of 0 means it is included, -1 means it is not*/
int checkIfIntervalIn(struct makerTime interval1,struct makerTime interval2){
	int ret = 2;
	if(interval1.hours < interval2.hours){
		ret--;
	}else if(interval1.hours > interval2.hours){
		return -1;
	}else if(interval1.mins < interval2.mins){
		ret--;
	}else if(interval1.mins > interval2.mins){
		return -1;
	}else if(interval1.secs < interval2.secs){
		ret--;
	}else if(interval1.secs > interval2.secs){
		return -1;
	}else if(interval1.msecs <= interval2.msecs){
		ret--;
	}else if(interval1.msecs > interval2.msecs){
		return -1;
	}

	if(interval1.endHours > interval2.endHours){
		ret--;
	}else if(interval1.endHours < interval2.endHours){
		return -1;
	}else if(interval1.endMins > interval2.endMins){
		ret--;
	}else if(interval1.endMins < interval2.endMins){
		return -1;
	}else if(interval1.endSecs > interval2.endSecs){
		ret--;
	}else if(interval1.endSecs < interval2.endSecs){
		return -1;
	}else if(interval1.endMsecs >= interval2.endMsecs){
		ret--;
	}else if(interval1.endMsecs < interval2.endMsecs){
		return -1;
	}

	return ret;
}