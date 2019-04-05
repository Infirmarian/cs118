
CC=g++
FLAGS=-Wall -Wextra

build: utils.o server.o
	$(CC) $(FLAGS) -o server server.o utils.o

server.o: server.cpp
	$(CC) $(FLAGS) -c server.cpp

utils.o: utils.cpp
	$(CC) $(FLAGS) -c utils.cpp

clean:
	$(RM) server.o
	$(RM) utils.o
	$(RM) server