all: exe1 exe2

exe1: src/sender.cpp
	g++ src/sender.cpp -std=c++11 -pthread -o sendfile

exe2: src/recv.cpp
	g++ src/recv.cpp -std=c++11 -pthread -o recvfile
