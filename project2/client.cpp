//
//  client.cpp
//  project2
//
//  Created by Robert Geil on 5/3/19.
//  Copyright © 2019 Robert Geil. All rights reserved.
//

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>

#include "packet.hpp"

int main(int argc, char** argv){
    if(argc != 4){
        std::cerr<<"Bad arguments, expected \n./client [HOSTNAME] [PORT] [FILENAME]"<<std::endl;
        exit(1);
    }
    
    // Set host name, port number, and file name
    char *hostname = argv[1];
    int port = atoi(argv[2]);
    if(port <= 0){
        std::cerr<<"Bad port number "<<port<<" provided"<<std::endl;
        exit(1);
    }
    char *filename = argv[3];

    // Create the socket file descriptor
    int socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd < 0) {
        std::cerr<<"Could not create socket"<<std::endl;
        exit(2);
    }

    // Set the server address
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));

    // Set all of the server information
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    
    // TODO: Deal with the translation of a possible DNS name to IP address
    if(strcmp(hostname, "localhost") == 0)
        strcpy(hostname, "127.0.0.1");

    serveraddr.sin_addr.s_addr = inet_addr(hostname);
    
    if(connect(socketfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0){
        std::cerr<<"Failed to connect to server"<<std::endl;
        exit(2);
    }
    
    
      ////////////////////////////////////////
     ///////// Handle TCP setup /////////////
    ////////////////////////////////////////
    
    // Generate a randomized initial sequence number
    int sequence_number = random() % MAX_SEQ;
    Packet* syn = new Packet(sequence_number, 0, false, true, false); // New packet with only the Syn bit sent
    
    // Send the SYN message to server, and await a response
    if(syn->sendPacket(socketfd) == -1){
        std::cerr<<"Failed to send initial SYN bit packet: "<<strerror(errno)<<std::endl;
        exit(2);
    }
    Packet* ack = new Packet(socketfd, 0, 0);
    ack->toString();
    
    // Check if the server accepted or rejected the connection
    if(!ack->SYNbit() || !ack->getAckNumber()){
        std::cerr<<"Server rejected the connection :/"<<std::endl;
        exit(3);
    }
    
    // Open the file to be tranmitted to the server!
    int fd = open(filename, O_RDONLY);
    if(fd == -1){
        std::cerr<<"Unable to open provided file: "<<strerror(errno)<<std::endl;
        exit(2);
    }
    
    
    
    delete(syn);
    delete(ack);
    close(fd);
    close(socketfd);
}
