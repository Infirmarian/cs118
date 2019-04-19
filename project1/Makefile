
CC=g++
FLAGS=-Wall -Wextra -g -std=c++11

build: utils.o server.o HttpRequest.o HttpResponse.o File.o
	$(CC) $(FLAGS) -o server server.o utils.o HttpRequest.o HttpResponse.o File.o

check: utils.o tests.o
	$(CC) $(FLAGS) -o test tests.o utils.o
	./test

# Note ID should be an environmental variable
dist: server.cpp utils.cpp utils.h HttpRequest.cpp HttpRequest.h HttpResponse.cpp HttpResponse.h tests.cpp \
		README File.h File.cpp 404.html index.html
	tar -czf $(ID).tar.gz server.cpp utils.cpp utils.h HttpRequest.cpp HttpRequest.h \
	HttpResponse.cpp HttpResponse.h tests.cpp README File.h File.cpp 404.html index.html

server.o: server.cpp
	$(CC) $(FLAGS) -c server.cpp

utils.o: utils.cpp
	$(CC) $(FLAGS) -c utils.cpp

HttpRequest.o: HttpRequest.cpp
	$(CC) $(FLAGS) -c HttpRequest.cpp

HttpResponse.o: HttpResponse.cpp
	$(CC) $(FLAGS) -c HttpResponse.cpp

File.o: File.cpp
	$(CC) $(FLAGS) -c File.cpp

junk: junk.cpp
	$(CC) $(FLAGS) junk.cpp -o junk
clean:
	$(RM) server.o
	$(RM) utils.o
	$(RM) server
	$(RM) HttpRequest.o
	$(RM) HttpResponse.o
	$(RM) tests.o
	$(RM) File.o
	$(RM) test
	$(RM) junk
	$(RM) $(ID).tar.gz