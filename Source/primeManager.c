
/* Code from https://github.com/Ph-k/Concurrent-Primes. Philippos Koumparos (github.com/Ph-k)*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>
#include "utilities.h"
#include "list.h"

char* typeToProgram(int Wnum){//Given the serial number of a W procces the function returns the prime program that has to be used 
	switch((Wnum%3)+1){//The primes programs are used cyclically, Wnum%3+1 returns a number in [1,3] cyclically as Wnum increases
		case 1:
			return "./prime1";
		case 2:
			return"./prime2";
		case 3:
			return "./prime3";
		default:
			printf("switch error\n");//Error case, unreachable
			return NULL;
	}	
}

int main(int argc, char *argv[]){
	if(argc!=7){ printf("Process manager args %d\n",argc); return -1;} //The prime manager program needs 6 arguments, which are:
	int minnum=atoi(argv[2]),//1. First number of range
		maxnum=atoi(argv[3]),//2. Last number of range
		fatherFd=atoi(argv[4]),//3. The pipe the root node created for the communication with this inner node
		numOfChildren=atoi(argv[5]),//4. The number of procceses the midlle procceses create
		Inum=atoi(argv[1]),//5. The serial number of the inner node. And 6. The pid of the root process
		Wnum=Inum*numOfChildren,//Calculating the first serial number of the frist Wnode of this inner node
		slice=(maxnum-minnum)/numOfChildren,//'slice' represents how many numbers each W node will take as input (approximately)
		index=minnum,
		fd[2],pid,i;

	char *program=NULL,*writePipe=NULL,
		  //lb,ub string will have the numbers wich represent the range of the W nodes.
		 *lb=malloc(sizeof(char*)*digitsCount(maxnum)+2),//The strings are big enough to save the maxnun of the range of this inner node
		 *ub=malloc(sizeof(char*)*digitsCount(maxnum)+2);//Thus any value of W ranges can be saved in those strings

	struct pollfd *pfds=malloc(sizeof(struct pollfd)*numOfChildren);//Initializing the arrey of pipe file descriptors for poll()
	
	slice-=(slice>0)? 1 : 0; //In order for the ranges to be as equal as possible, the slice has to decrease if possible
	for(i=0; i<numOfChildren; i++){
		
		//The ranges for the W nodes given this inner node range, are calculated exactly in the same manner as in the myPrime.c
		sprintf(lb,"%d",index);
		if(numOfChildren-1!=i && slice + index < maxnum){
			index += slice;
		}else{
			index = maxnum;
		}
		sprintf(ub,"%d",index);
		index+=(index<maxnum)? 1 : 0; //Increasing start of the next range if possible

		if( pipe(fd) == -1) return -3; //Creating the file discriptors
		
		pid = fork();//Forking the program to generate a new process
		if(pid == -1){
			return -2;//The program will terminate, if any unexpected error occurs
		}else if(pid == 0){//Child part...
			close(fd[0]);//The child will only write in the pipe, thus the reading pipe is not needed and gets closed
			myToString(&writePipe,fd[1]);//Converting the write file discriptor to string, to be passed as argument
			program = typeToProgram(Wnum);//Given the serial number of this W node, the right program will be executed in order for the programs to be chosen cyclically
			
			execlp(program,program,lb,ub,writePipe,argv[6], NULL);//Executing the right prime finding program to create the W node
			printf("something went wrong %s\n",program);
			return -2;//The program will terminate, if any unexpected error occurs
		}else{//Parrent part...
			Wnum++;//Incrising the serial number of W's for the next W node
			close(fd[1]);//The parrent will only read from the pipe, thus the writing pipe is not needed and gets closed
			pfds[i].fd=fd[0];//Saving the pipe value in the arrey poll() will use
			pfds[i].events = POLLIN;//Saving the use of the pipe for poll()
		}
		
	}
	
	
	list *primelist=initialize();//Initializing list which will compose the prime numbers randromly received from W nodes to a increasing sequence
	int flag = numOfChildren;
	struct numberTime prime;
	struct pipeMessage message;
	while(flag!=0){//While there are still pipes open that might send messages
		if(poll(pfds, numOfChildren, -1) == -1) {printf("myprime manager\n"); return -3;}//Poll is used to "wait" until one or more pipes have data
		for(i=0; i<numOfChildren; i++){//For all the pipes
			if(pfds[i].revents & POLLIN){//If according to poll() the pipe has data to be red
				read(pfds[i].fd,&message,sizeof(struct pipeMessage));//The data/message is red
				if(message.type==0){//type=0 means that this pipe message has prime & time
					prime.number = message.number;
					prime.time = message.time;
					insertSorted(primelist,prime);//After the prime and the message are extracted from the message struct, the prime and the number are inserted to the list of primes sorted
				}else if(message.type==1){//type=1 means that this pipe message has total execution time of this W node
					message.number = Inum*numOfChildren+i;//The serial number of the W node is added to the message
					if( write(fatherFd,&(message),sizeof(struct pipeMessage)) == -1 ) {printf("write from manager\n"); return -1;}//Sending the time message to the root
				}else if(message.type==-1){//type -1 is the last message from the pipe since the pipe will now close from the child
					close(pfds[i].fd);//Closing file discriptor
					flag--;//Reducing the number of open pipes
				}				
			}
		}

	}
	
	listNode* tlist=primelist->head;//The list of prime numbers has been composed
	message.type = 0;
	while(tlist!=NULL){//For all the int double tuples of the list
		//The prime and time are placed in a message of type 0 and are sent to the root
		message.number = tlist->numTime.number;
		message.time = tlist->numTime.time;
		//Sending the prime and the number using pipe (the data of the list are sent serialy)
		if( write(fatherFd,&(message),sizeof(struct pipeMessage)) == -1 ) {printf("write from manager\n"); return -1;}
		tlist=tlist->next;
	}
	
	message.type=-1;//Sending message to root that this pipe will now close
	if( write(fatherFd,&message,sizeof(struct pipeMessage)) == -1 ) {printf("write from manager\n"); return -1;}
	if( close(fatherFd) == -1 ) {printf("close from manager\n"); return -1;}
	
	//Freeing all the memory that has been dynamically allocated
	free(lb);
	free(ub);
	free(pfds);
	if(writePipe!=NULL) free(writePipe);
	destroy(primelist);
	return 0;
}