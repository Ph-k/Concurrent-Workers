/* Code from https://github.com/Ph-k/Concurrent-Workers. Philippos Koumparos (github.com/Ph-k)*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/times.h>
#include <signal.h>
#include "utilities.h"

int main(int argc, char *argv[]){
	if(argc!=5){ printf("Invalid args"); return -1;}//The same arguments as prime1 are given at the prime3
	int minnum=atoi(argv[1]),maxnum=atoi(argv[2]),fatherFd=atoi(argv[3]),n,flag,i;
	
	struct pipeMessage prime;//Initializing message tuple
	prime.type=0;
	
	struct tms startb, endb;
	double ticspersec = (double) sysconf(_SC_CLK_TCK),start,end;
	start = (double) times(&startb);//Starting time
	/*This prime finding program checks if a number is prime by checking if the number is divisible with multiples of prime numbers
	But in order for the above to work, the numbers need to be greater than 7, so if there are numbers smaller than 7 in the given range
	and need to be checked, the numbers up to 7 are checked using a differnt algorithm*/
	
	flag = (maxnum>=7)? 7 : maxnum;//Setting the range for the different algorithm that checks the numbers up to 7 (or all the numbers, if the range is smaller than 7)
	for(n=minnum; n<=flag; n++){

		if(n==2 || n==3 || n==5 || n==7){//It is faster to check if the given number is one of the prime numbers up to the value of 7, than checking with divisions
			end = (double) times(&endb);
			prime.number=n;
			prime.time=(end - start) / ticspersec;//Time needed for calculation
			if( write(fatherFd,&prime,sizeof(struct pipeMessage)) == -1 ) {printf("write from prime3\n"); return -1;}//Sending prime and time to primeManager using pipe
		}
	}

	//The next prime number after 7 is 11, so if the start of the range is smaller than 7 the algorithm will start from 11, 
	//otherwise the algorithm will start from the start of the range.
	if (minnum<11) minnum=11;


	for(n=minnum|1; n<=maxnum; n=n+2){ /* bitwise or ensures we always start with an odd number since the only even-prime number is 2 */		
		
		flag=0; 
		/*since the number is odd therefore not divasible by 2 or its multiplied (4,8,10 ect.) */
		if (n%3!=0){	/* we continue by serching if the number we are cheking is divisable by 3,*/
			for (i=5; i*i<=n; i=i+6){ /*or by 5 and 7 and their multiplied*/ 
				if(n%i==0 || n%(i+2)==0){ /*unlit the squrt of the number, because there is no need to check further ( squrt(n)=i => i*i=n ) */
					flag++; /*if the number is divisable by 5 and 7 and their multiplied we change the value of flag*/
					break; /* and we end the loop since we found a number exept 1 and it's self
							wich can divide it, so the number is not prime*/
				}
			}
		}else{ /*if n%3==0 (means number is divasible by 3)*/		
			flag++; /*we change the value of flag, since 3 is out of range
					and we found a number exept 1 and it's self
					wich can divide it, so the number is not prime*/
		}
		if (flag==0){ /*if the value of flag hasn't change,*/
					/*it means that we didn't find any number wich can divide it (exept 1 and it self) wich we are not cheking baucase all number are devidable with 1 and there self exept 0 */
			       /*so the number is prime and ...*/
			end = (double) times(&endb);//...We take the time needed to find the prime
			prime.number=n;//The prime it self
			prime.time=(end - start) / ticspersec;
			if( write(fatherFd,&prime,sizeof(struct pipeMessage)) == -1 ) {printf("write from prime3\n"); return -1;}//And sending the number and time using pipe
		}
	}

	end = (double) times(&endb);//End time of the procces
	prime.time=(end - start) / ticspersec;
	prime.type=1;//Changing the message type value, to send the total execution time
	if( write(fatherFd,&prime,sizeof(struct pipeMessage)) == -1 ) {printf("write from prime3\n"); return -1;}
	prime.type=-1;//Changing the message type value, to signal the closing of the pipe
	if( write(fatherFd,&prime,sizeof(struct pipeMessage)) == -1 ) {printf("write from prime3\n"); return -1;}
	if( close(fatherFd) == -1 ) {printf("close from prime3\n"); return -1;}
	const union sigval dummy;//Dummy value for sigqueue
	sigqueue(atoi(argv[4]),SIGUSR1,dummy);//Sending usr1 signal to root
}