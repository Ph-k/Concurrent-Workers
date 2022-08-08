
/* Code from https://github.com/Ph-k/Concurrent-Primes. Philippos Koumparos (github.com/Ph-k)*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <poll.h>
#include <errno.h>
#include "utilities.h"
#include "list.h"

int usr1c = 0;
struct sigaction sa;
void usr1Counter(int signum){
	usr1c++;
}

int main(int argc, char *argv[]){
	sa.sa_handler = usr1Counter;
	sigaction(SIGUSR1,&sa,NULL);//Setting handling function for usr1 singal (singal() could be used but sigaction is more reliable)

	int minnum=-1,maxnum=-1,numOfChildren=-1,i;
	
	if(argc!=7) {printf("myprime: not enough arguments!");}//My prime needs 6 arguments:

	for(i=1; i<argc; i++){
		if( strcmp(argv[i],"-l") == 0 ){
			minnum = atoi(argv[i+1]);//-l followed by the starting number of the prime finding range
		}else if( strcmp(argv[i],"-u") == 0){
			maxnum = atoi(argv[i+1]);//-u followed by the last number of the prime finding range
		}else if( strcmp(argv[i],"-w") == 0){
			numOfChildren = atoi(argv[i+1]);//-w followed by the number of processes the root and each inner process has to create
		}
		i++;
	}
	
	//All the arguments have to be possitive, since no negative or 0 prime exist, and the processes need to create at least one process
	if(minnum<=0 || maxnum<=0 || numOfChildren<=0){
		printf("myprime: one or more of '-l' '-u' '-w' arguments where mistyped or not given at all! (check readme for format)\n");
		return -3;
	}
	
	if(maxnum<minnum){//Error checking for range given from user
		printf("myprime: Invalid range, -l (%d) must be smaller or equal to -u (%d)\n",minnum,maxnum);
		return -3;
	}

	if( maxnum-minnum+1 < numOfChildren*numOfChildren){//Error checking for valid number of processes given from the user
		printf("myprime: More processeses than primes!, can't check %d numbers using %d prime finding processes\n",maxnum-minnum+1,numOfChildren*numOfChildren);
		return -3;
	};
	
	int slice=(maxnum-minnum)/numOfChildren,//'slice' represents how many numbers each inner node will take as input (approximately)
		index=minnum,fd[2],pid;
	
	//lb,ub string will have the numbers wich represent the range of the inner nodes. Since only strings can be given as arguments in execlp
	char *lb=malloc(sizeof(char*)*digitsCount(maxnum)+2),//The strings are big enough to save the maxnum of the range of the program
		 *ub=malloc(sizeof(char*)*digitsCount(maxnum)+2),//Thus any value of inner node ranges can be saved in those strings
		 *Inum=NULL,*writePipe=NULL,*cNum=NULL,*parentId=NULL;
	
	struct pollfd *pfds=malloc(sizeof(struct pollfd)*numOfChildren);//Initializing the arrey of pipe file descriptors for poll()

	if(minnum%10==0) slice-=(slice>0)? 1 : 0; //In order for the ranges to be as equal as possible, the slice has to decrease if possible
	for(i=0; i<numOfChildren; i++){
		
		sprintf(lb,"%d",index);//Start of the range of the inner node
		if((numOfChildren-1!=i && slice + index < maxnum) ){//If there will be more inner nodes to give numbers and the range does not exceed the maxnum
			index += slice;//The end of range differs from the starting range 'slice' numbers, since 'slice' is a value to lead to an even distribution of ranges
		}else{
			index = maxnum; //Otherwise, the end of the range has to be maximum number, in order to check only the requested numbers
		}
		sprintf(ub,"%d",index);//The end of index is saved as a string to be passed using exec(), and the starting range of the next processes (index++) will start from the next number
		index+=(index<maxnum)? 1 : 0; //Increasing start of the next range if possible

		if( pipe(fd) == -1) return -3; //Creating the file discriptors for this inner process
		
		pid = fork();//Forking the program to generate a new process
		if(pid == -1){
			return -2;//The program will terminate, if any unexpected error occurs
		}else if(pid == 0){//Child part...
			close(fd[0]);//The child will only write in the pipe, thus the reading pipe is not needed and gets closed
			
			//Converting the number arguments of the exec to strings, since numbers cannot be given as arguments
			myToString(&writePipe,fd[1]);//Write file discriptor to string using custom function implimented on utilities.h
			myToString(&cNum,numOfChildren);//Number of procceses each process has to create
			myToString(&parentId,getppid());//The id of the root procces
			myToString(&Inum,i);//The serial number of the inner node
			
			execlp("./primeManager", "./primeManager",Inum,lb,ub,writePipe,cNum,parentId, NULL);//The inner node is created using exec and the according program
			printf("something went wrong myprime\n");
			return -2;//The program will terminate, if any unexpected error occurs
		}else{//Parrent part...
			close(fd[1]);//The parrent will only read from the pipe, thus the writing pipe is not needed and gets closed
			pfds[i].fd=fd[0];//Saving the pipe value in the arrey poll() will use
			pfds[i].events = POLLIN;//Saving the use of the pipe for poll()
		}
	}
	
	list **result = malloc(sizeof(list*)*numOfChildren),//An arrey in which the sorted lists of prime numbers from the inner nodes is saved
		 *wTimes = initialize();//A sorted list of W's execution times
	for(i=0; i<numOfChildren; i++){
		result[i]=initialize();//Initializing the list
	}
	
	int flag = numOfChildren;//This procces has created numOfChildren programs and pipes for communication
	struct numberTime prime,wTime;
	struct pipeMessage message;//This struct is how values are passed through pipes in int and double tuples, more on utilities.h line 3
	double maxLeafTime = -1.0,minLeafTime = -1.0;
	while(flag!=0){//While there are still pipes open that might send messages
		if( poll(pfds, numOfChildren, -1) == -1){//Poll is used to "wait" until one or more pipes have data, (non-blocking procedure as discussed in piazza)
			//Since this procces will receive usr1 singals, that singal might happen to be received when the process executes the poll()...
			//...leading to unexpected termination of the sysscall with a return value of -1
			//In order to control the behavior of the program in the above case, errno is used to check the cause of the termination of poll()
			if(errno == EINTR){//If the cause was EINTR, wich according to the man means "A signal was delivered before the time limit expired..."
				errno = 0;//We know a usr1 signal was received while we were executing poll and lead to an error, so re-initialize the errno value
				continue;//And we "restart" the loop and also poll() in order to contiue normaly with the pipe messages
			}else{
				printf("poll from myprime\n");//If the error in poll was not caused by the usr1 signal
				return -3;//Something really unexpected happend and the program will have to terminate
			}
		}
		for(i=0; i<numOfChildren; i++){//For all the pipes
			if(pfds[i].revents & POLLIN){//If according to poll() the pipe has data to be red
				read(pfds[i].fd,&message,sizeof(struct pipeMessage));//The data/message is red
				//and we do the following according to the data of the message (more on how message structure work on utilities.h line 3)
				if(message.type==0){//In sort type=0 means that this pipe message has...
					prime.number = message.number;//...a prime... 
					prime.time = message.time;//...and the time needed for its caluculation.
					insertLast(result[i],prime);//The inner nodes are responsible for compising the primes in a sorted list, the root just recreates the sorted list serialy (more on readme)
				}else if(message.type==1){//type=1 means that this pipe message has the id of an W node, and its total execution time
					if (maxLeafTime == -1.0 && minLeafTime == -1){//If this is the first time ever to be received
						maxLeafTime = message.time;//Min and Max times are initialized, since we can asume that the first value is both min and max in the beginning
						minLeafTime = message.time;
					}else if(maxLeafTime < message.time){
						maxLeafTime = message.time;//If a new bigger time is found it is the new maximum time of all the W nodes
					}else if(minLeafTime > message.time){
						minLeafTime = message.time;//If a new smaller time is found it is the new maximum time of all the W nodes
					}
					wTime.number = message.number;//serial number of W node
					wTime.time = message.time;//Time of W node
					insertSorted(wTimes,wTime);//Insertion in the list (the total times of the W nodes are kept sorted from the first W node to the last)
				}else if(message.type==-1){//type -1 is the last message from the pipe since the pipe will now close from the child
					close(pfds[i].fd);//So the parent also closed the read end
					flag--;//And the total number of active pipes is decreased
				}
			}
		}

	}
	
	flag=0;
	printf("Primes in [%d,%d] are:\n",minnum,maxnum);
	for(i=0; i<numOfChildren; i++){
		listNode* tlist=result[i]->head;
		while(tlist!=NULL){
			printf("%d in %f, ",tlist->numTime.number,tlist->numTime.time);//Printing all the primes numbers and their times (sorted from the list)
			tlist=tlist->next;
			flag++;//Total prime number counter
		}
	}printf("\nFound %d prime numbers in total\n\n",flag);

	printf("Min Time for Workers: %f msecs\n",minLeafTime*1000);
	printf("Max Time for Workers: %f msecs\n",maxLeafTime*1000);
	printf("Num of USR1 Received: %d/%d\n",usr1c,numOfChildren*numOfChildren);
	
	listNode* tlist=wTimes->head;
	while(tlist!=NULL){
		printf("Time for W%d: %f msec\n",tlist->numTime.number,(tlist->numTime.time)*1000);//Printing total times of W nodes, from first to last
		tlist=tlist->next;
	}

	//Freeing all the memory that has been dynamically allocated
	free(lb);
	free(ub);
	free(Inum);
	free(pfds);
	if(writePipe!=NULL) free(writePipe);
	if(cNum!=NULL) free(cNum);
	if(parentId!=NULL) free(parentId);
	for(i=0; i<numOfChildren; i++){
		destroy(result[i]);
	}
	free(result);
	destroy(wTimes);
	return 0;
}