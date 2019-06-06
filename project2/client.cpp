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
    std::queue<Packet*>* queue = ((pthread_packet*)data)->queue;
    int fd = ((pthread_packet*)data)->fd;
    while(1){
        mtx_outgoingQueue.lock();
        if(!queue->empty()){
            Packet* p = queue->front();
            if(p->sendPacket(fd) == -1){
                std::cerr<<"Unable to transmit packet: "<<strerror(errno)<<std::endl;
                p->toString();
                exit(2);
            }
            p->printSend(cwnd, ssthresh);
            p->setDuplicate();
            queue->pop();
            mtx_outgoingQueue.unlock();
        }else{
            mtx_outgoingQueue.unlock();
            usleep(100);
        }
    }
}

void* ReceiveAcks(void* data){
    std::queue<Packet*>* queue = ((pthread_packet*)data)->queue;
    int fd = ((pthread_packet*)data)->fd;
    while(1){
        Packet* p = new Packet(fd);
        mtx_ackReceivedQueue.lock();
        queue->push(p);
        mtx_ackReceivedQueue.unlock();
        usleep(100);
    }
}

void* RetransmissionHandle(void* data){
    std::queue<Packet*>* outgoingQueue = ((queue_cordination_packet*)data)->outgoingQueue;
    std::queue<Packet*>* incomingAcks = ((queue_cordination_packet*)data)->incomingAcks;
    std::set<Packet*>* inFlight = ((queue_cordination_packet*)data)->inflight;
    unsigned short highest_ack = MAX_SEQ + 1;
    unsigned short oldest_acked = MAX_SEQ + 1;
    while(1){
        // Go through the received queue and try to match with sent packets
        mtx_ackReceivedQueue.lock();
        bool ack_received = false;
        while(!incomingAcks->empty()){
            Packet* front = incomingAcks->front();
            // Initialize to the first received ACK
            if(oldest_acked > MAX_SEQ){
                oldest_acked = front->getAckNumber();
            }
            
            if(highest_ack != front->getAckNumber()){
                if(highest_ack > MAX_SEQ){
                    highest_ack = front->getAckNumber();
                }
                highest_ack += 512;
                ack_received = true;
            }
            delete front;
            incomingAcks->pop();
        }
        mtx_ackReceivedQueue.unlock();
        
        // Retransmit packets if no ACK was received in the specified amount of time
        if(ack_received){
            while(highest_ack != oldest_acked){
                if(inFlight->empty())
                    break;
                mtx_inflight.lock();
                for(std::set<Packet*>::iterator it = inFlight->begin(); it != inFlight->end(); it++){
                    unsigned short num = (*it)->getExpectedAckNumber();
                    if(num == oldest_acked){
                        oldest_acked = (*it)->getExpectedAckNumber() + (*it)->getPayloadSize();
                        // Clear out the packet
                        delete (*it);
                        inFlight->erase(it);
                        break;
                    }
                }
                mtx_inflight.unlock();
            }
        }else{
            mtx_outgoingQueue.lock();
            mtx_inflight.lock();
            for(auto it = inFlight->begin(); it != inFlight->end(); it++){
                outgoingQueue->push(*it);
            }
            mtx_outgoingQueue.unlock();
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

Packet* blockAndAwaitIncomingAck(std::queue<Packet*>* queue){
    while(1){
        mtx_ackReceivedQueue.lock();
        if(!queue->empty()){
            Packet* p = queue->front();
            queue->pop();
            mtx_ackReceivedQueue.unlock();
            return p;
        }
        mtx_ackReceivedQueue.unlock();
        usleep(10);
    }
}

Packet* clearFromInFlight(std::set<Packet*>* inflight, Packet* packet){
    mtx_inflight.lock();
    auto it = inflight->find(packet);
    if(it == inflight->end()){
        mtx_inflight.unlock();
        return 0;
    }
    Packet* p = *it;
    inflight->erase(it);
    mtx_inflight.unlock();
    return p;
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
    
    // Setup Multiple Threads to Send, Receive and Manage the two Queues
    std::queue<Packet*>* to_send = new std::queue<Packet*>();
    std::queue<Packet*>* received_acks = new std::queue<Packet*>();
    std::set<Packet*>* inFlight = new std::set<Packet*>();
    pthread_packet* p = new pthread_packet();
    p->queue = to_send;
    p->fd = socketfd;
    pthread_packet* rcv = new pthread_packet();
    rcv->queue = received_acks;
    rcv->fd = socketfd;
    
    queue_cordination_packet* qcp = new queue_cordination_packet();
    qcp->incomingAcks = received_acks;
    qcp->inflight = inFlight;
    qcp->outgoingQueue = to_send;
    
    pthread_t send_thread;
    pthread_t receive_thread;
    pthread_t queue_cordination;
    pthread_create(&send_thread, 0, TransmitPackets, p);
    pthread_create(&receive_thread, 0, ReceiveAcks, rcv);
    
    // Generate a randomized initial sequence number
    std::srand(std::time(0));
    unsigned short sequence_number = (unsigned short)(std::rand() % MAX_SEQ);
    Packet* syn = new Packet(sequence_number, 0, false, true, false); // New packet with only the Syn bit sent
    
    // Send the SYN message to server, and await a response
    queue_packet(syn, to_send, inFlight);

    // Receive ACK and print recv message
    Packet* ack = blockAndAwaitIncomingAck(received_acks);
    clearFromInFlight(inFlight, syn);
    
    ack->printRecv(cwnd, ssthresh);
    
    // Check if the server accepted or rejected the connection
    if(!ack->SYNbit() || !ack->getAckNumber() || !ack->ACKbit()){
        std::cerr<<"Server rejected the connection :/"<<std::endl;
        exit(3);
    }
    delete syn;
    
    // Open the file to be tranmitted to the server!
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
    queue_packet(initial_data, to_send, inFlight);




    // Set next sequence number
    if (file_size < 512) {
        sequence_number += file_size;
    } else {
        sequence_number += 512;
    }

    // ACK first packet
    ack = blockAndAwaitIncomingAck(received_acks);
    clearFromInFlight(inFlight, initial_data);
    ack->printRecv(cwnd, ssthresh);

    // Start joining the queue together
    pthread_create(&queue_cordination, 0, RetransmissionHandle, qcp);
    
    // TODO: should cwnd be increased on ACK of SYN?
    // SS / CA
    if (cwnd < ssthresh) {
        cwnd += 512;
    } else {
        cwnd += (512 * 512) / cwnd;
    }
    
    // Continue data transmission
    while (1) {
        mtx_inflight.lock();
        int inFlightCount = 512 * (int)inFlight->size();
        mtx_inflight.unlock();
        for(; inFlightCount<cwnd; inFlightCount += 512 ){
            Packet* p = new Packet(sequence_number, ack->getSequenceNumber(), 0,0,0);
            int incount = p->loadData(fd);
            queue_packet(p, to_send, inFlight);
            // Done reading in the file
            if(incount < 512){
                break;
            }
        }
        usleep(100);
    }

    // Teardown
    Packet* finpacket = new Packet(0,0,0,0,1);
    finpacket->sendPacket(socketfd);
    finpacket->printSend(cwnd, ssthresh);
    
    Packet* finack = new Packet(socketfd);
    if (finack->timeoutHit()) {
        close(socketfd);
        exit(5);
    }
    finack->printRecv(cwnd, ssthresh);

    delete(syn);
    delete(ack);
    delete(initial_data);
    close(fd);
    close(socketfd);
}
