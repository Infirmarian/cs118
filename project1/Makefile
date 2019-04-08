
CC=g++
FLAGS=-Wall -Wextra

build: utils.o server.o HttpRequest.o
	$(CC) $(FLAGS) -o server server.o utils.o HttpRequest.o

check: utils.o tests.o
	$(CC) $(FLAGS) -o test tests.o utils.o
	./test

server.o: server.cpp
	$(CC) $(FLAGS) -c server.cpp

utils.o: utils.cpp
	$(CC) $(FLAGS) -c utils.cpp

HttpRequest.o: HttpRequest.cpp
	$(CC) $(FLAGS) -c HttpRequest.cpp

clean:
	$(RM) server.o
	$(RM) utils.o
	$(RM) server
	$(RM) HttpRequest.o
	$(RM) tests.o
	$(RM) test