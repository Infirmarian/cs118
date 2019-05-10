//
//  server.cpp
//  project2
//
//  Created by Robert Geil on 5/3/19.
//  Copyright © 2019 Robert Geil. All rights reserved.
//

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <csignal>

#include "packet.hpp"


void signal_exit(int signum){
	(void) signum;
	_exit(0);
}

int main(int argc, char** argv){
    // Make sure arguments are valid
    if(argc != 2){
        std::cerr<<"Bad arguments, expected \n./server [PORT]"<<std::endl;
        exit(1);
    }
    // Set the port number
    int port = atoi(argv[1]);
    if(port <= 0){
        std::cerr<<"Bad port number "<<port<<" provided"<<std::endl;
        exit(1);
    }
	
	// Setup signal handler
	if(std::signal(SIGQUIT, signal_exit) == SIG_ERR || std::signal(SIGINT, signal_exit) == SIG_ERR){
		std::cerr<<"Unable to set a signal handler"<<std::endl;
		exit(2);
	}
	
    // Create the socket file descriptor
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd < 0) {
        std::cerr<<"Could not create socket"<<std::endl;
        exit(1);
    }

    // Set the server and client addresses
    struct sockaddr_in serveraddr, clientaddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    memset(&clientaddr, 0, sizeof(clientaddr));

    // Set all of the server information
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(port);

    // Attempt to bind the socket
    if (bind(socketfd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        std::cerr<<"Could not bind socket"<<std::endl;
        exit(2);
    }

	int connection_number = 0;
	
	// Continually listen and process input on the socket
	while(1){
		char buf[100];
		read(socketfd, buf, 100);
		std::cout<<buf<<std::endl;
	}
}
