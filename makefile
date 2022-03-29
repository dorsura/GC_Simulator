OBJS	= Auxilaries.o main.o
SOURCE	= Auxilaries.cpp main.cpp
HEADER	= Auxilaries.h FTL.hpp ListItem.h main.hpp MyRand.h AlgoRunner.h
OUT	= Simulator
CC	 = g++
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

Auxilaries.o: Auxilaries.cpp
	$(CC) $(FLAGS) Auxilaries.cpp -std=c++11

main.o: main.cpp
	$(CC) $(FLAGS) main.cpp -std=c++11


clean:
	rm -f $(OBJS) $(OUT)
