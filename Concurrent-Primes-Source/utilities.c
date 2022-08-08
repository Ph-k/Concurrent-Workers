
/* Code from https://github.com/Ph-k/Concurrent-Workers. Philippos Koumparos (github.com/Ph-k)*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int digitsCount(int num){
	if(num==0) return 1; //0 has only one digit
	int count=0;
	while(num!=0){
        num=num/10;//Each division with 10 resulting to a non zero value,
        count++;//Mean one more digit in our number
    }
	return count;
}

char* myToString(char **buffer,int num){
	int digits = digitsCount(num);//Calculating the digits of the number
	if(*buffer==NULL || strlen(*buffer)<=digits){//If the buffer does not have any allocated memory, or is not big enough
		if(*buffer!=NULL) free(*buffer); //Its allocated memory is freed
		*buffer = malloc(sizeof(char*)*digits+2);//And the needed memory is allocated
	}
	sprintf(*buffer,"%d",num);//The value of the number is placed using sprintf in the (100% sure) big enough buffer 
	return *buffer; //The pointer to the buffer is returned
}