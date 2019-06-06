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
#include <pthread.h>
#include <queue>
#include <mutex>

#include "packet.hpp"

std::mutex mtx_outgoingQueue;
std::mutex mtx_ackReceivedQueue;
std::mutex mtx_inflight;

struct pthread_packet{
    std::queue<Packet*>* queue;
    int fd;
};

struct queue_cordination_packet{
    std::queue<Packet*>* outgoingQueue;
    std::queue<Packet*>* incomingAcks;
    std::set<Packet*>* inflight;
};

static int cwnd = 512;
static int mincwnd = 512;
static int ssthresh = 5120;

void* TransmitPackets(void* data){
    std::queue<Packet*> queue = *((pthread_packet*)data)->queue;
    int fd = ((pthread_packet*)data)->fd;
    while(1){
        mtx_outgoingQueue.lock();
        if(!queue.empty()){
            Packet* p = queue.front();
            if(p->sendPacket(fd) == -1){
                std::cerr<<"Unable to transmit packet: "<<strerror(errno)<<std::endl;
                exit(2);
            }
            p->printSend(cwnd, ssthresh);
            p->setDuplicate();
            queue.pop();
            mtx_outgoingQueue.unlock();
        }else{
            mtx_outgoingQueue.unlock();
            usleep(100);
        }
    }
}

void* ReceiveAcks(void* data){
    std::queue<Packet*> queue = *((pthread_packet*)data)->queue;
    int fd = ((pthread_packet*)data)->fd;
    while(1){
        Packet* p = new Packet(fd, 0, 0);
        mtx_ackReceivedQueue.lock();
        queue.push(p);
        mtx_ackReceivedQueue.unlock();
    }
}

void* RetransmissionHandle(void* data){
    std::queue<Packet*> outgoingQueue = *((queue_cordination_packet*)data)->outgoingQueue;
    std::queue<Packet*> incomingAcks = *((queue_cordination_packet*)data)->incomingAcks;
    std::set<Packet*> inFlight = *((queue_cordination_packet*)data)->inflight;
    unsigned short highest_ack = MAX_SEQ + 1;
    unsigned short oldest_acked = MAX_SEQ + 1;
    while(1){
        // Go through the received queue and try to match with sent packets
        mtx_ackReceivedQueue.lock();
        bool ack_received = false;
        while(!incomingAcks.empty()){
            Packet* front = incomingAcks.front();
            // Initialize to the first received ACK
            if(oldest_acked > MAX_SEQ){
                oldest_acked = front->getAckNumber();
            }
            
            if(highest_ack != front->getAckNumber()){
                highest_ack = front->getAckNumber();
                ack_received = true;
            }
            delete front;
            incomingAcks.pop();
        }
        mtx_ackReceivedQueue.unlock();
        
        // Retransmit packets if no ACK was received in the specified amount of time
        if(!ack_received){
            mtx_inflight.lock();
            while(highest_ack != oldest_acked){
                for(std::set<Packet*>::iterator it = inFlight.begin(); it != inFlight.end(); it++){
                    if((*it)->getExpectedAckNumber() == oldest_acked){
                        oldest_acked = (*it)->getExpectedAckNumber() + (*it)->getPayloadSize();
                        // Clear out the packet
                        delete (*it);
                        inFlight.erase(it);
                        break;
                    }
                }
            }
            mtx_inflight.unlock();
        }
        // Await to process more ACKs
        usleep(500000);
    }
}

void queue_packet(Packet* pack, std::queue<Packet*>* transmission_queue, std::set<Packet*>* inflight){
    mtx_outgoingQueue.lock();
    mtx_inflight.lock();
    transmission_queue->push(pack);
    inflight->insert(pack);
    mtx_outgoingQueue.unlock();
    mtx_inflight.unlock();
}

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
    
    // Set localhost IP number
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
    std::queue<Packet*>* to_send = new std::queue<Packet*>();
    std::set<Packet*>* inFlight = new std::set<Packet*>();
    pthread_packet* p = new pthread_packet();
    p->queue = to_send;
    p->fd = socketfd;
    pthread_t send_thread;
    pthread_create(&send_thread, 0, TransmitPackets, p);
    
    // Generate a randomized initial sequence number
    std::srand(std::time(0));
    int sequence_number = std::rand() % MAX_SEQ;
    Packet* syn = new Packet(sequence_number, 0, false, true, false); // New packet with only the Syn bit sent
    
    // Send the SYN message to server, and await a response
    to_send->push(syn);
    inFlight->insert(syn);
    
    // Print send message
    syn->printSend(cwnd, ssthresh);

    // Receive ACK and print recv message
    Packet* ack = new Packet(socketfd, 0, 0);
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
    to_send->push(initial_data);
    inFlight->insert(initial_data);


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
    
    // Continue data transmission
    while (1) { 
        // Receive new acks from server for each unacked packet
        while (packets_sent > 0) {
            Packet* ackn = new Packet(socketfd, 0, 0);
            acks_recv = ackn->getAckNumber();
            
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
            if (sequence_number >= MAX_SEQ) {
                sequence_number = 0;
            }
            next_data->sendPacket(socketfd);
            // TODO: implement way to check if this is a duplicate packet
            next_data->printSend(cwnd, ssthresh);
            packets_sent++;
        }
    }

    // Teardown
    if(data_sent >= file_size){
        Packet* finpacket = new Packet(0,0,0,0,1);
        finpacket->sendPacket(socketfd);
        finpacket->printSend(cwnd, ssthresh);
        
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
