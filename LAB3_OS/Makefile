CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
LDFLAGS = -pthread -lrt

all: parent child

parent: parent.cpp
	$(CXX) $(CXXFLAGS) -o parent parent.cpp $(LDFLAGS)

child: child.cpp
	$(CXX) $(CXXFLAGS) -o child child.cpp $(LDFLAGS)

clean:
	rm -f parent child

.PHONY: all clean