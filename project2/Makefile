CC=g++
FLAGS=-Wall -Wextra -std=c++11

build: server client

client: client.cpp
	$(CC) $(FLAGS) client.cpp -o client

server: server.cpp
	$(CC) $(FLAGS) server.cpp -o server

clean:
	$(RM) client
	$(RM) server
