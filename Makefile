CXX := g++
CXXFLAGS := -std=c++17 -O3 -Wall -Wextra -Wpedantic -Iinclude
SRCS := src/main.cpp src/limitorderbook.cpp
TARGET := build/lobster

.PHONY: all run clean

all:
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -rf build
