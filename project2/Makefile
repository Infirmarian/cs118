CC=g++
FLAGS=-Wall -Wextra -std=c++11


build: client server

client: client.o
	$(CC) $(FLAGS) client.o -o client

server: server.o
	$(CC) $(FLAGS) server.o -o server

client.o: client.cpp
	$(CC) $(FLAGS) -c client.cpp
server.o: server.cpp
	$(CC) $(FLAGS) -c server.cpp

clean:
	$(RM) server.o
	$(RM) client.o
	$(RM) server
	$(RM) client
