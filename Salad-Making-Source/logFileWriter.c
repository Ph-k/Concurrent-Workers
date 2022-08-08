
/* Code from https://github.com/Ph-k/Concurrent-Workers. Philippos Koumparos (github.com/Ph-k)*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "list.h"

//This function simply creates (or deletes the old) needed log files for the given type of process
int initializeLog(char* pname){
	FILE *fp;
	if(strcmp("Chef",pname)==0)
		fp = fopen("mainLog.txt", "w+");
	else if(strcmp("SaladMaker1",pname)==0)
		fp = fopen("saladMaker1.txt", "w+");
	else if(strcmp("SaladMaker2",pname)==0)
		fp = fopen("saladMaker2.txt", "w+");
	else if(strcmp("SaladMaker3",pname)==0)
		fp = fopen("saladMaker3.txt", "w+");
	else 
		return -2;

	if(fp==NULL) return -1;
	fclose(fp);
	return 0;
}

//Given the process name (chef, saladmakerN), and a message. It is writen to the log at the time the write was requested
int writeToLog(char* pname,char* string){
	time_t curTime = time(NULL); //Getting current time/time at which the message was 'sent' to be writen

	//All the processes chef to makers, write to the main log, so it is opened in append mode to write at the end
	FILE *fp = fopen("mainLog.txt", "a+");
	if(fp==NULL) return -1;

	struct tm hours_mins_secs = *localtime(&curTime); //Local time is used to take the system hours, minutes, and seconds

	struct timeval msecs;
  	gettimeofday(&msecs, NULL); //gettimeofday is used to take the systems milliseconds

  	//Writing message in the log file, in the requsted format wit =h the requested data
	fprintf(fp,"[%02d:%02d:%02d.%ld] [%d] [%s] [%s]\n",
		hours_mins_secs.tm_hour, hours_mins_secs.tm_min, hours_mins_secs.tm_sec,msecs.tv_usec/10000,
		getpid(),pname,string);
	fclose(fp);//Closing the main log file, nothing needs to be write here

	if(strcmp("Chef",pname)==0)
		return 0;//If the message was writen on behalf of the chef, the function ends here, since the chef does not write in any other files
	else if(strcmp("SaladMaker1",pname)==0)//Otherwise, the messages on behalf of salad makers, are also written in their respective files
		fp = fopen("saladMaker1.txt", "a+");
	else if(strcmp("SaladMaker2",pname)==0)
		fp = fopen("saladMaker2.txt", "a+");
	else if(strcmp("SaladMaker3",pname)==0)
		fp = fopen("saladMaker3.txt", "a+");
	else 
		return -2;

	fprintf(fp,"[%02d:%02d:%02d.%ld] [%d] [%s] [%s]\n",
		hours_mins_secs.tm_hour, hours_mins_secs.tm_min, hours_mins_secs.tm_sec,msecs.tv_usec/10000,
		getpid(),pname,string);//Writing message to salad maker log file
	fclose(fp);//Closing salad maker log file
	return 0;
}

int* getMakersPIDFromLog(){
	int *makersPID = malloc(sizeof(int)*3);
	FILE *fp = fopen("mainLog.txt", "r");
	if(fp==NULL) return NULL;
	//                       This message does not exist, but is the maximum lenght a line in the log file be
	int maxString=strlen("[00:00:00:00] [0000000] [saladMaker3] [Selecting ingredients Green pepper Small onion]\n"),
		bracketCount,i,j1,j2;
	char *buffer = malloc(sizeof(char)*maxString),
		 *makerString=malloc(sizeof(char)*strlen("saladMaker3 ")),*makerInt=malloc(sizeof(char)*strlen("0000000 "));

	for(i=0; i<3; i++)
		makersPID[i]=-1;

	//While the log file still has line to be red, and not all the pid's are found
	while(fgets(buffer,sizeof(char)*maxString, fp) && (makersPID[0]==-1 || makersPID[1]==-1 || makersPID[2]==-1)){
		bracketCount=0;
		i=0;
		j1=0;
		j2=0;
		while(bracketCount<8){
			if(buffer[i]=='[' || buffer[i]==']'){
				bracketCount++;
			}else if(bracketCount==5){//The string of the saladmaker type start after the fith bracket and ends before the sixth
				makerString[j1++] = buffer[i];
			}else if(bracketCount==3){//The string of pif starts after the third bracket and ends before the fourth
				makerInt[j2++] = buffer[i];
			}
			i++;
		}
		//Placing string endings
		makerString[j1] = '\0';
		makerInt[j2] = '\0';

		if(makersPID[0]==-1 && strcmp("SaladMaker1",makerString)==0){
			makersPID[0]=atoi(makerInt); //If the id of saladmaker1 is found a log message it saved
		}else if(makersPID[1]==-1 && strcmp("SaladMaker2",makerString)==0){
			makersPID[1]=atoi(makerInt); //Same for saladmaker2
		}else if(makersPID[2]==-1 && strcmp("SaladMaker3",makerString)==0){
			makersPID[2]=atoi(makerInt); //And saladmaker3
		}
	}

	fclose(fp);//Closing lof file
	//Freeing the memory that was dynamically allocated for the strings
	free(makerString);
	free(buffer);
	free(makerInt);
	return makersPID; //Returning the arrey which containts the pid's of the saladmakers
}

int getTotalSaladsFromLog(int makerNum){
	if(makerNum>3) return -2;
	char *maker=malloc(sizeof(char)*17);
	sprintf(maker, "saladMaker%d.txt", makerNum); //Opening log file of the requested salad maker
	FILE *fp = fopen(maker, "r");
	free(maker);
	if(fp==NULL) return -1;
	int maxString=strlen("[00:00:00:00] [0000000] [saladMaker3] [Selecting ingredients Green pepper Small onion]\n"),bracketCount,i,j;
	char *buffer = malloc(sizeof(char)*maxString);
	char *message = malloc(sizeof(char)*maxString);
	int totalSalads = 0;

	while(fgets(buffer,sizeof(char)*maxString, fp)){

		bracketCount=0;
		i=0;
		j=0;
		while(bracketCount<8){
			if(buffer[i]=='[' || buffer[i]==']'){
				bracketCount++;
			}else if(bracketCount==7){
				message[j++] = buffer[i];//Reading the message of the log line
			}
			i++;
		}
		message[j] = '\0';
		if(strncmp(message,"End",3)==0){//If the log line indicates the end of a salad making
			totalSalads++;//Total number of salads made from this maker is increased
		}
	}

	fclose(fp);
	free(buffer);
	free(message);
	return totalSalads;
}

int getTimeIntervalsFromLog(list *list,int makerNum){
	if(makerNum>3) return -2;
	char *maker=malloc(sizeof(char)*17);
	sprintf(maker, "saladMaker%d.txt", makerNum); //Opening log file of the requested salad maker
	FILE *fp = fopen(maker, "r");
	free(maker);
	if(fp==NULL) return -1;
	struct makerTime time;
	int maxString=strlen("[00:00:00:00] [0000000] [saladMaker3] [Selecting ingredients Green pepper Small onion]\n"),bracketCount,i,j,
				  hours,mins,secs,msecs;
	char *buffer = malloc(sizeof(char)*maxString);

	char *action = malloc(sizeof(char)*maxString);

	while(fgets(buffer,sizeof(char)*maxString, fp)){

		//Reading time
		sscanf(buffer,"[%02d:%02d:%02d.%02d]",&(hours), &(mins), &(secs), &(msecs));

		bracketCount=0;
		i=0;
		j=0;
		while(bracketCount<8){
			if(buffer[i]=='[' || buffer[i]==']'){
				bracketCount++;
			}else if(bracketCount==7){
				action[j++] = buffer[i]; //The action/message of the log lie is brackets 7 & 8
			}
			i++;
		}
		action[j] = '\0';		

		//Note, since only the salad maker writes in his log file, the start and end log messages are always writen in the correct order

		if(strncmp(action,"Start",5)==0){
			//If this was a starting time, it is saved as the start of the interval
			time.maker = makerNum;
			time.hours = hours;
			time.mins = mins;
			time.secs = secs;
			time.msecs = msecs;
		}else if(strncmp(action,"End",3)==0){
			//If this was an end time, it is save as the end of the interval
			time.endHours = hours;
			time.endMins = mins;
			time.endSecs = secs;
			time.endMsecs = msecs;
			insertLast(list,time);//And the interval is inserted in the list of intervals
		}
	}

	fclose(fp);
	free(buffer);
	free(action);
	return 0;
}