#pragma once
#include <time.h>

typedef struct list list; //Forward declaration

int initializeLog(char* pname); //Function creating all the needed log files
int writeToLog(char* pname,char* string); //Function is writing the message to the needed log files
int* getMakersPIDFromLog(); //Returns an arrey of integers, which represent the pid's of the salad maker processes (information taken from log file)
int getTotalSaladsFromLog(int makerNum); //Returns how many salads, a salad maker has created based on the information in his log file
int getTimeIntervalsFromLog(list *list,int makerNum);//Populates the given list with all the salad making times for the given salad maker