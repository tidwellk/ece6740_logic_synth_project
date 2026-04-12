CXX = g++
CXXFLAGS = -Wall -Wextra -g -std=c++20 -fsanitize=address -fsanitize=undefined
TARGET = main.out
OBJS = main.o SolutionState.o Assignment.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

run: $(TARGET)
	./$(TARGET) f.txt

main.o: main.cpp SolutionState.h Assignment.h Val.h
	$(CXX) $(CXXFLAGS) -c main.cpp

SolutionState.o: SolutionState.cpp SolutionState.h Assignment.h Val.h
	$(CXX) $(CXXFLAGS) -c SolutionState.cpp

Assignment.o: Assignment.cpp Assignment.h Val.h
	$(CXX) $(CXXFLAGS) -c Assignment.cpp

clean:
	rm -fv $(TARGET) *.o

.PHONY: all run clean
