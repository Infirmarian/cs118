
CC=g++
FLAGS=-Wall -Wextra

build: utils.o server.o HttpObject.o
	$(CC) $(FLAGS) -o server server.o utils.o HttpObject.o

check: utils.o tests.o
	$(CC) $(FLAGS) -o test tests.o utils.o
	./test

server.o: server.cpp
	$(CC) $(FLAGS) -c server.cpp

utils.o: utils.cpp
	$(CC) $(FLAGS) -c utils.cpp

HttpObject.o: HttpObject.cpp
	$(CC) $(FLAGS) -c HttpObject.cpp

clean:
	$(RM) server.o
	$(RM) utils.o
	$(RM) server
	$(RM) HttpObject.o
	$(RM) tests.o
	$(RM) test