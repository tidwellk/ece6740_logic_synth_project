CC = g++
CFLAGS = -Wall -fsanitize=undefined -g -fsanitize=address -std=c++20

main: main.o matrixcover.o
	$(CC) $(CFLAGS) -o main.out main.o matrixcover.o

run: main
	./main.out f.txt

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

matrixcover.o: matrixcover.cpp
	$(CC) $(CFLAGS) -c matrixcover.cpp

clean:
	rm -fv main.out *.o