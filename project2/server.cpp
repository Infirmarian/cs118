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
#include <cstdlib>
#include <ctime>

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
	
    // Create the incoming socket file descriptor
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd < 0) {
		std::cerr<<"Could not create socket: "<<strerror(errno)<<std::endl;
        exit(1);
    }

    // Set the server and client addresses
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
    memset(&serveraddr, 0, sizeof(serveraddr));

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
	socklen_t addr_len;
	byte buf[HEAD_LENGTH + DATA_LENGTH];

	// Continually listen and process new connections on the socket
	while(connection_number == 0){
		  ///////////////////////////////////////////
		 /////// SET UP INCOMING CONNECTION ////////
		///////////////////////////////////////////
		memset(&clientaddr, 0, sizeof(clientaddr));
		addr_len = sizeof(clientaddr);
		long bytes_read = recvfrom(socketfd, buf, 524, 0, (struct sockaddr *) &clientaddr, &addr_len);
		if(bytes_read == -1){
			std::cerr<<"Error reading in from connection: "<<strerror(errno)<<std::endl;
			continue;
		}
		// Convert read in bytes to a Packet object
		Packet* p = new Packet(buf, (short)bytes_read);
		p->toString();

		// No new connection to set up
		if(! p->SYNbit())
			continue;
		
		
		if(connect(socketfd, (struct sockaddr *) &clientaddr, sizeof(clientaddr)) < 0){
			std::cerr<<"Failed to connect to client: "<<strerror(errno)<<std::endl;
			exit(2);
		}
		
		// Setting up a new connection
		std::srand((unsigned) std::time(0) - 10101);
		int server_seqnum = (std::rand() + std::rand()) % MAX_SEQ;
		// SYNACK message (handshake part II)
		Packet* ack = new Packet(server_seqnum, p->getSequenceNumber()+1, 1, 1, 0);
		if(ack->sendPacket(socketfd) == -1){
			exit(2);
		}
		
		// Listen for next packets
		while(1) {
			// Listen for next packet
			Packet* next_data = new Packet(socketfd);
			next_data->toString();
			
			// Check for closed connection
			if (next_data->FINbit())
				break;
			
			// Send ack to client
			Packet* ackn = new Packet(server_seqnum, next_data->getSequenceNumber()+1, 1, 0, 0);
			if(ackn->sendPacket(socketfd) == -1){
				exit(2);
			}
		}

		connection_number++;
	}
	
}
