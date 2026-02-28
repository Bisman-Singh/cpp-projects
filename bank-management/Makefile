CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2

TARGET = bank

all: $(TARGET)

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) main.cpp

clean:
	rm -f $(TARGET) accounts.dat

.PHONY: all clean
