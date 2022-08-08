#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/times.h>
#include <signal.h>
#include "utilities.h"

/* Code from https://github.com/Ph-k/Concurrent-Workers. Philippos Koumparos (github.com/Ph-k)*/

#define YES 1
#define NO  0

int prime(int n){//The second given prime finding algorithm
        int i=0, limitup=0;
        limitup = (int)(sqrt((float)n));

        if (n==1) return(NO);
        for (i=2 ; i <= limitup ; i++)
                if ( n % i == 0) return(NO);
        return(YES);
}

//The main of prime2 was the same as the main of prime1 in the given code, so the same modifications where made, and the same comments apply here
int main(int argc, char *argv[]){
        int lb=0, ub=0, i=0 ,fatherFd;

        if(argc!=5){ printf("Invalid args"); return -1;}

        lb=atoi(argv[1]);
        ub=atoi(argv[2]);
		fatherFd=atoi(argv[3]);
		struct pipeMessage primeMes;
		primeMes.type=0;
		struct tms startb, endb;
		double ticspersec = (double) sysconf(_SC_CLK_TCK),start,end;
		start = (double) times(&startb);
		
        if ( ( lb<1 )  || ( lb > ub ) ) {
                printf("usage: prime2 lb ub\n");
                exit(1); }

        for (i=lb ; i <= ub ; i++){
                if ( prime(i)==YES ){
						end = (double) times(&endb);
						primeMes.number=i;
						primeMes.time=(end - start) / ticspersec;
						if( write(fatherFd,&primeMes,sizeof(struct pipeMessage)) == -1 ) {printf("write from prime2\n"); return -1;}
				}
		}

		end = (double) times(&endb);
		primeMes.time=(end - start) / ticspersec;
		primeMes.type=1;
		if( write(fatherFd,&primeMes,sizeof(struct pipeMessage)) == -1 ) {printf("write from prime2\n"); return -1;}
		primeMes.type=-1;
		if( write(fatherFd,&primeMes,sizeof(struct pipeMessage)) == -1 ) {printf("write from prime2\n"); return -1;}
		if( close(fatherFd) == -1 ) {printf("close from prime2\n"); return -1;}
		const union sigval dummy;
		sigqueue(atoi(argv[4]),SIGUSR1,dummy);
}
