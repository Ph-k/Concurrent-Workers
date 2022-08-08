#pragma once
#include <time.h>

#define START 1
#define END 0

/*The info of the listnode, along with the start and the end time of the salad making
also has information on which salad maker this time and action concerns*/
struct makerTime{
	//Start time
	int hours;
	int mins;
	int secs;
	int msecs;

	//End time
	int endHours;
	int endMins;
	int endSecs;
	int endMsecs;

	//Which maker 1,2,3
	int maker;
};

struct listNode{ //Each node of the list has the above tuple of values and a pointer to the next node (NULL if there is no next)
	struct makerTime time;
	struct listNode *next;
};
typedef struct listNode listNode;

struct list{ //The list data structure has:
	listNode *head; //a pointer to its beggining
	listNode *tail; //and a pointetr to its end, needed for insertion to end of the list in O(1)
};
typedef struct list list;

list* initialize(); //Function which initializes the data structure and returning a pointer to its head and tail nodes
int insertSorted(list* list,struct makerTime time); //Creates a new listNode and inserts to a sorted position
int insertLast(list* list,struct makerTime start); //Creates a new listNode and inserts it in the end of the list in O(1)
int destroy(list* listS); //Frees all the memory the data structure used
//Functions for comparing the time in our makerTime struct
int compareTimeSt(struct makerTime time1,struct makerTime time2); 
int compareTime(int hours1,int mins1, int secs1, int msecs1,int hours2,int mins2, int secs2, int msecs2);