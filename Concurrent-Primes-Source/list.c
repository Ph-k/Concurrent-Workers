#include <stdlib.h>
#include "utilities.h"
#include "list.h"

/* Code from https://github.com/Ph-k/Concurrent-Workers. Philippos Koumparos (github.com/Ph-k)*/

list* initialize(){//The initialization creates an empty lits
	list* listS = malloc(sizeof(list));
	//An empty list has no node in both its start and its end
	listS->head=NULL;
	listS->tail=NULL;
	return listS;
}

int insertSorted(list* listS,struct numberTime numTime){//Inserting sorted with regard on the number value of numTime
	listNode* newNode = malloc(sizeof(listNode)),**head=&(listS->head),**tail=&(listS->tail);
	newNode->numTime=numTime;//Creating new node and copying the int and double values
	newNode->next=NULL;
	
	if(*head == NULL){//If the list is empty
		*head=newNode;//The new node is both the end and start
		*tail=newNode;
		return 0;//End of function
	}else if((*head)->numTime.number > newNode->numTime.number){//If the list is not empty, but the new node needs to be placed first
		newNode->next=*head;//The first value is not the first anymore
		*head=newNode;//And the value of the "head" needs to be updated
		return 0;//End of function
	}
	listNode *temp=*head;
	do{//Otherwise, the new node must be inserted somewhere in the list
		if(temp->next==NULL || temp->next->numTime.number > newNode->numTime.number){//If the sorted position of the new node is found
			newNode->next=temp->next; //The new is placed before the found bigger value, or last if no such value was found
			temp->next=newNode;//The new node points to the next bigger value, or NULL if it is the bigest 
			if(temp==*tail) *tail = newNode;//If the newnode is placed after the tail, it is the new tail
			return 0;//End of function
		}
		temp=temp->next;//Next node for iteration
	}while(temp!=NULL);
	return -1;//Something went wrong, if no position was found for the node in the whole list
}

int insertLast(list* listS,struct numberTime numTime){
	listNode* newNode = malloc(sizeof(listNode)),**head=&(listS->head),**tail=&(listS->tail);
	newNode->numTime=numTime;//Creating new node and copying the int and double values
	newNode->next=NULL;
	
	if(*head==NULL){//If the list is empty
		*head=newNode;//The new node is both the end and start
		*tail=newNode;
		return 0;
	}else{//Otherwise the new node must be placed last
		(*tail)->next=newNode;//Thus the next node of previous last node is the newnode
		*tail=newNode;//And the current last node is the new node
		return 0;
	}
	return -1;//If the node was not inserted in any of the above cases something went wrong
}

int destroy(list *listS){ //In order to free all the memory allocated from the list
	listNode *temp,*head=listS->head;
	while(head!=NULL){
		temp=head;
		head=head->next;
		free(temp);//All the memory of the nodes has to be freed
	}
	free(listS);//And also the momory of the structure which had the start and the end of the list
	return 0;
}