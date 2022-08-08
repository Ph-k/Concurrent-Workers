#pragma once

/* Code from https://github.com/Ph-k/Concurrent-Workers. Philippos Koumparos (github.com/Ph-k)*/

/*Struct to save the value of an int and a double, 
used to hold the values of a prime and the time needed to find it, 
or a Worker id and it's total execution time*/
struct numberTime{ 
	int number;
	double time;
};


struct listNode{ //Each node of the list has the above tuple of values and a pointer to the next node (NULL if there is no next)
	struct numberTime numTime;
	struct listNode *next;
};
typedef struct listNode listNode;

struct list{ //The list data structure has:
	listNode *head; //a pointer to its beggining
	listNode *tail; //and a pointetr to its end, needed for insertion to end of the list in O(1)
};
typedef struct list list;

list* initialize(); //Function which initializes the data structure and returning a pointer to its head and tail nodes
int insertSorted(list* list,struct numberTime numTime); //Creates a new listNode and inserts to a sorted position with regard to the number of the numTime tuple
int insertLast(list* listS,struct numberTime numTime); //Creates a new listNode and inserts it in the end of the list in O(1)
int destroy(list* listS); //Frees all the memory the data structure used