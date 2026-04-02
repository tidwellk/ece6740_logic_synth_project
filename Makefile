CC = g++
CFLAGS = -Wall -fsanitize=undefined -g -fsanitize=address -std=c++20

main: main.o SolutionState.o
	$(CC) $(CFLAGS) -o main.out main.o SolutionState.o

run: main.out
	./main.out f.txt

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

SolutionState.o: SolutionState.cpp
	$(CC) $(CFLAGS) -c SolutionState.cpp

clean:
	rm -fv main.out *.o