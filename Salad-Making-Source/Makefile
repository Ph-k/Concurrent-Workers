# Code from https://github.com/Ph-k/Concurrent-Workers. Philippos Koumparos (github.com/Ph-k)

CC = gcc
cflags = -Wall -g -pthread

myLibs = MySharedMemLib.o logFileWriter.o 
dataStructures = list.o
chefObjects = chef.o 
saladObjects = saladMakers.o

chefProgram = chef
saladProgram = saladmaker

all: $(chefObjects) $(saladObjects)
	$(CC) $(cflags) $(myLibs) $(dataStructures) $(chefObjects) -o $(chefProgram)
	$(CC) $(cflags) $(myLibs) $(dataStructures) $(saladObjects) -o $(saladProgram)

$(chefProgram): $(chefObjects)
	$(CC) $(cflags) $(myLibs) $(dataStructures) $(chefObjects) -o $(chefProgram)

$(saladProgram): $(saladObjects)
	$(CC) $(cflags) $(myLibs) $(dataStructures) $(saladObjects) -o $(saladProgram)
	
chef.o: chef.c $(myLibs) $(dataStructures)
	$(CC) $(cflags) -c chef.c

saladMakers.o: saladMakers.c $(myLibs) $(dataStructures) $(dataStructures)
	$(CC) $(cflags) -c saladMakers.c

MySharedMemLib.o: MySharedMemLib.c
	$(CC) $(cflags) -c MySharedMemLib.c

logFileWriter.o: logFileWriter.c
	$(CC) $(cflags) -c logFileWriter.c

list.o: list.c
	$(CC) $(cflags) -c list.c

run: $(chefProgram) $(saladProgram)
	./$(chefProgram) -n 10 -m 2

clean:
	rm -f $(chefObjects) $(saladObjects) $(myLibs) $(chefProgram) $(saladProgram) $(dataStructures)

dclean:
	rm -f $(chefObjects) $(saladObjects) $(myLibs) $(chefProgram) $(saladProgram) $(dataStructures) mainLog.txt saladMaker1.txt saladMaker2.txt saladMaker3.txt
