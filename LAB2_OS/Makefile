CXX = g++
CXXFLAGS = -O2 -Wall -pthread

lab: main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o lab

run: lab
	./lab -k 100 -n 1000000 -t 16

clean:
	rm -f lab