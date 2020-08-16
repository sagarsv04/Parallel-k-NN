# Make file for building application

CC = g++
CFLAGS = -Wall -Wextra -std=c++11 -lpthread -pedantic -O3 -ldl -pthread

k-nn: k-nn.o bst.o
	$(CC) k-nn.o bst.o $(CFLAGS) -o k-nn

k-nn.o: k-nn.cpp bst.hpp
	$(CC) -c k-nn.cpp bst.hpp $(CFLAGS)

bst.o: bst.cpp
	$(CC) -c bst.cpp $(CFLAGS)

clean:
	rm -f *.o *.d k-nn
