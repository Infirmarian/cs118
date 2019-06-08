CC=g++
FLAGS=-Wall -Wextra -std=c++11 -g


build: client server

client: client.o packet.o
	$(CC) $(FLAGS) client.o packet.o -o client

server: server.o packet.o
	$(CC) $(FLAGS) server.o packet.o -o server

dist: server.cpp client.cpp packet.cpp packet.hpp report.pdf README Makefile
	zip project2_104916969_104934015.zip server.cpp client.cpp packet.cpp packet.hpp report.pdf README Makefile

client.o: client.cpp
	$(CC) $(FLAGS) -pthread -c client.cpp
server.o: server.cpp
	$(CC) $(FLAGS) -c server.cpp
packet.o: packet.cpp
	$(CC) $(FLAGS) -c packet.cpp

clean:
	$(RM) server.o
	$(RM) client.o
	$(RM) packet.o
	$(RM) server
	$(RM) client
	$(RM) project2_104916969_104934015.zip
