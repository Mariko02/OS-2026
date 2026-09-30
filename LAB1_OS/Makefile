CXX = g++
CXXFLAGS = -Wall -Wextra

TEST_FILES = ok.txt one.txt spaces.txt nolf.txt zero.txt bad.txt edge.txt big.txt minus.txt large.txt

all: parent child

parent: parent.cpp
	$(CXX) $(CXXFLAGS) -o parent parent.cpp

child: child.cpp
	$(CXX) $(CXXFLAGS) -o child child.cpp

test: all
	@printf "100 5 2\n81 3 3\n-20 4\n" > ok.txt
	@printf "42\n" > one.txt
	@printf "\n   100    10  \n\n20 2\n" > spaces.txt
	@printf "100 5\n50 2" > nolf.txt
	@printf "100 5 2\n81 3 3\n-20 4\n7 0 1\n50 2\n" > zero.txt
	@printf "100 abc\n" > bad.txt
	@printf "7 2\n-7 2\n100 -5 2\n0 5\n100\t5\r\n" > edge.txt
	@printf "99999999999 1\n" > big.txt
	@printf "5 -\n" > minus.txt
	@for i in $$(seq 1 2000); do echo "100 5 2"; done > large.txt

	@echo "=== ok.txt (expect 10 9 -5) ==="
	@echo "--- input:"
	@cat ok.txt
	@echo "--- output:"
	-@printf "ok.txt\n" | ./parent
	@echo

	@echo "=== one.txt (expect 42) ==="
	@echo "--- input:"
	@cat one.txt
	@echo "--- output:"
	-@printf "one.txt\n" | ./parent
	@echo

	@echo "=== spaces.txt (expect 10 10) ==="
	@echo "--- input:"
	@cat -A spaces.txt
	@echo "--- output:"
	-@printf "spaces.txt\n" | ./parent
	@echo

	@echo "=== nolf.txt (expect 20 25) ==="
	@echo "--- input (no newline at the end):"
	@cat nolf.txt
	@echo
	@echo "--- output:"
	-@printf "nolf.txt\n" | ./parent
	@echo

	@echo "=== zero.txt (expect 10 9 -5, then division by zero) ==="
	@echo "--- input:"
	@cat zero.txt
	@echo "--- output:"
	-@printf "zero.txt\n" | ./parent
	@echo

	@echo "=== bad.txt (expect invalid number) ==="
	@echo "--- input:"
	@cat bad.txt
	@echo "--- output:"
	-@printf "bad.txt\n" | ./parent
	@echo

	@echo "=== edge.txt (expect 3 -3 -10 0 20) ==="
	@echo "--- input:"
	@cat -A edge.txt
	@echo "--- output:"
	-@printf "edge.txt\n" | ./parent
	@echo

	@echo "=== big.txt (expect out of range) ==="
	@echo "--- input:"
	@cat big.txt
	@echo "--- output:"
	-@printf "big.txt\n" | ./parent
	@echo

	@echo "=== minus.txt (expect invalid number) ==="
	@echo "--- input:"
	@cat minus.txt
	@echo "--- output:"
	-@printf "minus.txt\n" | ./parent
	@echo

	@echo "=== large.txt (expect: 2000 10) ==="
	@echo "--- input: 2000 lines of '100 5 2'"
	@echo "--- output (count and value):"
	-@printf "large.txt\n" | ./parent | sort | uniq -c
	@echo

	@echo "=== nofile.txt (expect open error) ==="
	@echo "--- input: file name 'nofile.txt', file does not exist"
	@echo "--- output:"
	-@printf "nofile.txt\n" | ./parent
	@echo

	@echo "=== empty name (expect empty name error) ==="
	@echo "--- input: empty line instead of file name"
	@echo "--- output:"
	-@printf "\n" | ./parent

clean:
	rm -f parent child $(TEST_FILES)