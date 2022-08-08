/* Code from https://github.com/Ph-k/Concurrent-Workers. Philippos Koumparos (github.com/Ph-k)*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>
#include <signal.h>
#include "utilities.h"

#define YES 1
#define NO  0

int prime(int n){//The first given prime finding algorithm
        int i;
        if (n==1) return(NO);
        for (i=2 ; i<n ; i++)
                if ( n % i == 0) return(NO);
        return(YES);
}

int main(int argc, char *argv[]){
        int lb=0, ub=0, i=0 ,fatherFd;

        if(argc!=5){ printf("Invalid args"); return -1;}//A W node needs 4 arguments 

        lb=atoi(argv[1]);//First argument is the start of the number range
        ub=atoi(argv[2]);//Second argument is the end of the number range
		fatherFd=atoi(argv[3]);//The id of the root node is given in the third argument
		struct pipeMessage primeMes;//Initializing variable tuple which is the standard to send values through pipe (more on utilities.h)
		primeMes.type=0;//As noted in utilities.h the message type shows what the values in the struct reprent
		struct tms startb, endb;
		double ticspersec = (double) sysconf(_SC_CLK_TCK),start,end;
		start = (double) times(&startb);//Starting time of the execution
		
        if ( ( lb<1 )  || ( lb > ub ) ) {//The values of the range are checked (given code, the primeManager ensures the arguments are well given)
                printf("usage: prime1 lb ub\n");
                exit(1); }

        for (i=lb ; i <= ub ; i++){//Finding prime numbers, given code
                if ( prime(i)==YES ){
						end = (double) times(&endb);//Time in which the prime was found
						//Composing the message to send the prime number and the time needed
						primeMes.number=i;//The prime
						primeMes.time=(end - start) / ticspersec;//The total time needed for its calculations
						//Sending prime, ending execution if unexpected errors occur
						if( write(fatherFd,&primeMes,sizeof(struct pipeMessage)) == -1 ) {printf("write from prime2\n"); return -1;}
				}
		}

		end = (double) times(&endb);//End time of procces execution
		primeMes.time=(end - start) / ticspersec;
		primeMes.type=1;//Message type 1, sends the total time of execution
		if( write(fatherFd,&primeMes,sizeof(struct pipeMessage)) == -1 ) {printf("write from prime1\n"); return -1;}
		primeMes.type=-1;//Message type -1, last message from procces through the pipe, since the procces will now close the pipe
		if( write(fatherFd,&primeMes,sizeof(struct pipeMessage)) == -1 ) {printf("write from prime1\n"); return -1;}
		if( close(fatherFd) == -1 ) {printf("close from prime2\n"); return -1;}
		const union sigval dummy;//Dummy value for sigqueue
		sigqueue(atoi(argv[4]),SIGUSR1,dummy);//Sending usr1 signal to root (kill could be used, but sigqueue is more reliable)
}
