# Code from https://github.com/Ph-k/Concurrent-Workers. Philippos Koumparos (github.com/Ph-k)

CC = gcc
mainObjects = myPrime.o 
dependencies = utilities.o list.o
managerObjects = primeManager.o
primeObjects = prime1.o prime2.o prime3.o
cflags = -Wall -g
mainProgram = myprime

$(mainProgram): $(mainObjects) $(managerObjects) $(primeObjects) $(dependencies)
	gcc $(mainObjects) $(dependencies) -o $(mainProgram)
	gcc $(managerObjects) $(dependencies) -o primeManager
	gcc prime1.o -o prime1
	gcc prime2.o -o prime2 -lm
	gcc prime3.o -o prime3
	
myPrime.o: myPrime.c
	$(CC) $(cflags) -c myPrime.c

utilities.o: utilities.c
	$(CC) $(cflags) -c utilities.c

primeManager.o: primeManager.c
	$(CC) $(cflags) -c primeManager.c

prime1.o: prime1.c
	$(CC) $(cflags) -c prime1.c

prime2.o: prime2.c
	$(CC) $(cflags) -c prime2.c

prime3.o: prime3.c
	$(CC) $(cflags) -c prime3.c

list.o: list.c
	$(CC) $(cflags) -c list.c

run: $(mainProgram)
	./$(mainProgram) -l 1 -u 1000 -w 3

clean:
	rm -f $(mainProgram) primeManager prime1 prime2 prime3
	rm -f $(mainObjects) $(managerObjects) $(primeObjects) $(dependencies)
