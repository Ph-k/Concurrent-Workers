#pragma once

/* Code from https://github.com/Ph-k/Concurrent-Primes. Philippos Koumparos (github.com/Ph-k)*/

struct pipeMessage{ //This structure represents the data that are passed through a pipe
	int number; //An integer (regarding the, type this can be a prime number or the id of a W process node)
	double time; //An double (regarding the, type this can be the time taken to find a prime number or the time take by a W process node)
	int type; /*The type can have 3 values, each sets what the above values represent:
	type=0 means that this pipe message has a prime and the time needed for its caluculation 
	type=1 means that this pipe message has the id of an W node, and its total execution time
	type=-1 signals the last message of this pipe, since it will now close, tha values of number time do not represent something and can be anything
	*/
};

int digitsCount(int num); //It returns who many digits the given intern has
char* myToString(char **buffer,int num); //Copies the given integer vaue to the given buffer (reallocates memory if the size is not enough)