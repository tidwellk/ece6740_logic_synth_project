CXX = g++
CXXFLAGS = -Wall -Wextra -g -std=c++20 -fsanitize=address -fsanitize=undefined
TARGET = main.out
OBJS = main.o SolutionState.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

run: $(TARGET)
	./$(TARGET) f.txt

test: $(TARGET)
	for file in *.txt; do ./$(TARGET) "$$file"; done

main.o: main.cpp SolutionState.h Val.h main.h
	$(CXX) $(CXXFLAGS) -c main.cpp

SolutionState.o: SolutionState.cpp SolutionState.h Val.h
	$(CXX) $(CXXFLAGS) -c SolutionState.cpp

clean:
	rm -fv $(TARGET) *.o

.PHONY: all run test clean
