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
#include <cstdlib>
#include <ctime>
#include <set>
#include <unordered_map>
#include <chrono>
#include <csignal>

#include "packet.hpp"

void timeout_handle(int sign){
    std::cerr<<"\nTimer expired, need to retransmit packet!"<<std::endl;
    exit(14);
}

int main(int argc, char** argv){
    int cwnd = 512;
    int mincwnd = 512;
    int ssthresh = 5120;
    
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
    
    // Set localhost IP number
    if(strcmp(hostname, "localhost") == 0)
        strcpy(hostname, "127.0.0.1");

    serveraddr.sin_addr.s_addr = inet_addr(hostname);
    
    if(connect(socketfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0){
        std::cerr<<"Failed to connect to server"<<std::endl;
        exit(2);
    }
    
    // Setup signal handler
    if(std::signal(SIGALRM, timeout_handle) == SIG_ERR){
        std::cerr<<"Unable to set signal handler: "<<strerror(errno)<<std::endl;
    }
    
      ////////////////////////////////////////
     ///////// Handle TCP setup /////////////
    ////////////////////////////////////////
    
    // Generate a randomized initial sequence number
    std::srand(std::time(0));
    int sequence_number = std::rand() % MAX_SEQ;
    Packet* syn = new Packet(sequence_number, 0, false, true, false); // New packet with only the Syn bit sent
    
    // Send the SYN message to server, and await a response
    if(syn->sendPacket(socketfd) == -1){
        std::cerr<<"Failed to send initial SYN bit packet: "<<strerror(errno)<<std::endl;
        exit(2);
    }
    // Print send message
    syn->printSend(cwnd, ssthresh, false);

    // Receive ACK and print recv message
    Packet* ack = new Packet(socketfd, 0, 0);
    if (ack->timeoutHit()) {
        close(socketfd);
        exit(5);
    }
    ack->printRecv(cwnd, ssthresh);
    
    // Check if the server accepted or rejected the connection
    if(!ack->SYNbit() || !ack->getAckNumber() || !ack->ACKbit()){
        std::cerr<<"Server rejected the connection :/"<<std::endl;
        exit(3);
    }
    
    // Open the file to be tranmitted to the server!
    // std::cout<<"Filename: "<<filename<<std::endl;
    int fd = open(filename, O_RDONLY);
    if(fd == -1){
        std::cerr<<"Unable to open provided file: "<<strerror(errno)<<std::endl;
        exit(2);
    }
    long file_size = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET); // Reset file position
    
    // Begin data transmission
    int data_sent = 0;
    Packet* initial_data = new Packet(++sequence_number, ack->getSequenceNumber()+1, 1,0,0);
    data_sent += initial_data->loadData(fd);
    initial_data->sendPacket(socketfd);

    // TODO: increase CWND before print???
    // Print send message
    initial_data->printSend(cwnd, ssthresh, false);

    // Set next sequence number
    if (file_size < 512) {
        sequence_number += file_size;
    } else {
        sequence_number += 512;
    }

    // Initialize ACKed bytes variable
    int acks_recv = 0;

    // Initialize packets in the air counter (initially 1 because initial packet sent)
    int packets_sent = 1;

    // Flag to check if all packets ACKed
    bool all_acked_flag = false;

    // ACK first packet
    Packet* ackn = new Packet(socketfd, 0, 0);
    if (ackn->timeoutHit()) {
        close(socketfd);
        exit(5);
    }
    acks_recv = ackn->getAckNumber();
    ackn->printRecv(cwnd, ssthresh);

    // Check if only one packet to send
    if (data_sent >= file_size) {
        all_acked_flag = true;
    }
    // TODO: should cwnd be increased on ACK of SYN?
    // SS / CA
    if (cwnd < ssthresh) {
        cwnd += 512;
    } else {
        cwnd += (512 * 512) / cwnd;
    }

    packets_sent--;
    
    std::set<Packet*> inFlight;
    std::unordered_map<int, Packet*> inFlightHash;
    // Continue data transmission
    while (1) { 
        // Receive new acks from server for each unacked packet
        while (packets_sent > 0) {
            Packet* ackn = new Packet(socketfd, 0, 0);
            if (ackn->timeoutHit()) {
                close(socketfd);
                exit(5);
            }
            acks_recv = ackn->getAckNumber();
            /*
            auto sent = inFlightHash.find(acks_recv);
            if(sent == inFlightHash.end()){
                std::cerr<<"Retrieved packet not in sent map!"<<std::endl;
            }
            if(*(inFlight.begin()) == sent->second){
                Packet* second = *std::next(inFlight.begin(), 1);
                if(second == *(inFlight.end())){
                    ualarm(0,0); // Reset the alarm
                }else{
                    inFlight.erase(sent->second);
                    long long time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - second->getCreationTime();
                    if(time >= 500000){
                        time = 450000;
                    }
                    ualarm(500000 - time, 500000);
                    inFlightHash.erase(acks_recv);
                }
            }*/
            ualarm(500000, 500000);
            
            
            ackn->printRecv(cwnd, ssthresh);
            if (data_sent >= file_size && acks_recv >= sequence_number) {
                all_acked_flag = true;
                break;
            }
            if (cwnd < 10240) {
                if (cwnd < ssthresh) {
                    cwnd += 512;
                } else {
                    cwnd += (512 * 512) / cwnd;
                }
            } else {
                cwnd = 10240;
            }
            packets_sent--;
            delete (ackn);
        }

        // If the whole file has been sent and ACKed, break out of loop
        if (all_acked_flag) {
            break;
        } 

        // Send next packets
        int window_limit = data_sent+cwnd;
        while (data_sent < window_limit && data_sent < file_size) {
            Packet* next_data = new Packet(sequence_number, ackn->getSequenceNumber()+1, 1,0,0);
            data_sent += next_data->loadData(fd);
            sequence_number += next_data->getPayloadSize();
            // Set an alarm to go off every 0.5 seconds
            if(inFlight.size() == 0){
                ualarm(500000, 500000);
            }
            inFlight.insert(next_data);
            inFlightHash.insert(std::pair<int, Packet*>((next_data->getSequenceNumber()+next_data->getPayloadSize()) % MAX_SEQ, next_data));
            if (sequence_number >= MAX_SEQ) {
                sequence_number = 0;
            }
            next_data->sendPacket(socketfd);
            // TODO: implement way to check if this is a duplicate packet
            next_data->printSend(cwnd, ssthresh, false);
            packets_sent++;
        }
    }

    // Teardown
    if(data_sent >= file_size){
        Packet* finpacket = new Packet(0,0,0,0,1);
        finpacket->sendPacket(socketfd);
        finpacket->printSend(cwnd, ssthresh, false);
        
        Packet* finack = new Packet(socketfd);
        if (finack->timeoutHit()) {
            close(socketfd);
            exit(5);
        }
        finack->printRecv(cwnd, ssthresh);
    }

    delete(syn);
    delete(ack);
    delete(initial_data);
    close(fd);
    close(socketfd);
}
