CXX=g++
CXXFLAGS=-O3 -std=c++11
TARGET=main

SRC=$(shell ls *.cc)

all: $(TARGET)

$(TARGET): $(SRC:.cc=.o)
	$(CXX) $(CXXFLAGS) $+ -o $@

-include $(SRC:.cc=.d)

clean:
	$(RM) *.o *.d $(TARGET)

.PHONY: all clean

