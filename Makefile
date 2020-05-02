DIROBJ := obj/
DIREXE := exec/
DIRSRC := src/
DIRBOOKS := books/
DIRHEA := include/

INC := include/color.h include/msgRequest.h include/SemCounter.h

CFLAGS :=  -I$(DIRHEA) -c  -pthread -std=c++11
CC := g++

all : dirs msgRequest SemCounter cinema main

dirs:
	mkdir -p $(DIROBJ) $(DIREXE)

msgRequest: 
	$(CC) -o $(DIROBJ)msgRequest.o $(DIRSRC)msgRequest.cpp $(CFLAGS) 

SemCounter: 
	$(CC) -o $(DIROBJ)SemCounter.o $(DIRSRC)SemCounter.cpp $(CFLAGS) 

cinema: 
	$(CC) -o $(DIROBJ)cinema.o $(DIRSRC)cinema.cpp $(CFLAGS) 

main:
	$(CC) -o $(DIREXE)cinema $(DIROBJ)cinema.o $(DIROBJ)msgRequest.o $(DIROBJ)SemCounter.o -pthread -std=c++11

run:
	./$(DIREXE)cinema
	
clean:
	rm -r $(DIREXE) $(DIROBJ)