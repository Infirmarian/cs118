//
//  server.cpp
//  project2
//
//  Created by Robert Geil on 5/3/19.
//  Copyright © 2019 Robert Geil. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <unistd.h>
#include <csignal>
#include <cstdlib>
#include <ctime>

#include "packet.hpp"

// Set as -1 if not writing to file currently
// Otherwise should be set as file number
// Used for interrupt signal
int open_file = -1;

void signal_exit(int signum){
	(void) signum;
	if (open_file > 0) {
		std::cout<<std::to_string(open_file)<<std::endl;
		std::ofstream outfile (std::to_string(open_file) + ".file", std::ofstream::trunc);
		const char* msg = "INTERRUPT";
		outfile.write(msg, sizeof(msg)+1);
		outfile.close();
	}
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

    // Set the server and client addresses
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
    memset(&serveraddr, 0, sizeof(serveraddr));

    // Set all of the server information
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(port);

	int connection_number = 1;
	socklen_t addr_len;
	byte buf[HEAD_LENGTH + DATA_LENGTH];

	// Continually listen and process new connections on the socket
	while(1){
		// Create the incoming socket file descriptor
    	int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    	if (socketfd < 0) {
			std::cerr<<"Could not create socket: "<<strerror(errno)<<std::endl;
        	exit(1);
    	}

		// Attempt to bind the socket
    	if (bind(socketfd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0) {
        	std::cerr<<"Could not bind socket"<<std::endl;
        	exit(2);
    	}

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
		if (p->timeoutHit()) {
        	std::ofstream outfile(std::to_string(connection_number) + ".file", std::ofstream::trunc);
			outfile.close();
			close(socketfd);
			connection_number++;
			continue;
    	}
		p->printRecv(0, 0);

		// No new connection to set up
		if(! p->SYNbit())
			continue;
		
		if(connect(socketfd, (struct sockaddr *) &clientaddr, sizeof(clientaddr)) < 0){
			std::cerr<<"Failed to connect to client: "<<strerror(errno)<<std::endl;
			exit(2);
		}
		
		// Setting up a new connection
		std::srand(std::time(0));
    	int server_seqnum = std::rand() % MAX_SEQ;
		// SYNACK message (handshake part II)
		Packet* ack = new Packet(server_seqnum, p->getSequenceNumber()+1, 1, 1, 0);
		ack->printSend(0, 0, false);
		server_seqnum++;
		if (server_seqnum >= MAX_SEQ) {
			server_seqnum = 0;
		}
		if(ack->sendPacket(socketfd) == -1){
			exit(2);
		}

		// Define output file
		std::ofstream outfile(std::to_string(connection_number) + ".file", std::ofstream::trunc);
		open_file = connection_number;

		// Flag for timeout
		bool timeout_occurs = false;
		
		// Listen for next packets
		while(1) {
			// Listen for next packet
			Packet* next_data = new Packet(socketfd);
			if (next_data->timeoutHit()) {
				timeout_occurs = true;
				break;
    		}
			next_data->printRecv(0, 0);

			// Write data to file
			outfile.write((char*)next_data->getData(), next_data->getPayloadSize());
			
			// Check for closed connection
			if (next_data->FINbit())
				break;
			
			// Send ack to client
			Packet* ackn = new Packet(server_seqnum, next_data->getSequenceNumber()+next_data->getPayloadSize(), 1, 0, 0);
			ackn->printSend(0, 0, false);
			if(ackn->sendPacket(socketfd) == -1){
				exit(2);
			}
		}
		// Send FIN packet before closing connection
		if (!timeout_occurs) {
			Packet* finpacket = new Packet(0,0,0,0,1);
			finpacket->sendPacket(socketfd);
			finpacket->printSend(0, 0, false);
		}
		
		outfile.close();
		open_file = -1;
		close(socketfd);
		connection_number++;
	}
}