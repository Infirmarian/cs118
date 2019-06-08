//
//  client.cpp
//  project2
//
//  Created by Robert Geil on 5/3/19.
//  Copyright © 2019 Robert Geil. All rights reserved.
//

#include <iostream>
#include <algorithm>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <csignal>
#include <pthread.h>
#include <queue>
#include <mutex>
#include <vector>
#include <string>
#include <cstring>

#include "packet.hpp"

#define MAX_CWND 10240
#define TIMEOUT 500000

std::mutex mtx_outgoingQueue;
std::mutex mtx_ackReceivedQueue;
std::mutex mtx_inflight;
std::mutex mtx_printlock;
bool finished_transmission = false;
bool all_acked = false;
unsigned short ackNumber = -1;
int socketfd;
int fd;
int last_expected_ack = MAX_SEQ * 2;

struct pthread_packet{
    std::queue<Packet*>* queue;
    int fd;
};

struct queue_cordination_packet{
    std::queue<Packet*>* outgoingQueue;
    std::queue<Packet*>* incomingAcks;
    std::unordered_map<unsigned short, Packet*>* inflight;
};

static int cwnd = 512;
static int ssthresh = 5120;

void* TransmitPackets(void* data){
    std::queue<Packet*>* queue = ((pthread_packet*)data)->queue;
    int fd = ((pthread_packet*)data)->fd;
    int finished_og_transmission = false;
    while(!finished_og_transmission){
        mtx_outgoingQueue.lock();
        if(!queue->empty()){
            Packet* p = queue->front();
            if(p->sendPacket(fd) == -1){
                std::cerr<<"Unable to transmit packet: "<<std::strerror(errno)<<std::endl;
                exit(2);
            }
            mtx_printlock.lock();
            p->printSend(cwnd, ssthresh);
            mtx_printlock.unlock();
            //p->setDuplicate();
            queue->pop();
            mtx_outgoingQueue.unlock();
        }else{
            mtx_outgoingQueue.unlock();
            usleep(10);
        }
        finished_og_transmission = finished_transmission;
    }
    return 0;
}

void* ReceiveAcks(void* data){
    std::queue<Packet*>* queue = ((pthread_packet*)data)->queue;
    int fd = ((pthread_packet*)data)->fd;
    bool fin = false;
    while(1){
        if (finished_transmission)
            fin = true;
        Packet* p = new Packet(fd, fin);
        if(p->timeoutHit()){
            delete p;
            close(socketfd);
            close(fd);
            _exit(5);
        }
        if(p->getAckNumber() == last_expected_ack)
            all_acked = true;
        mtx_printlock.lock();
        if(p->FINbit()){
            p->printRecv(0, 0);
            Packet* p = new Packet(0,0,1,0,0);
            p->sendPacket(socketfd);
            delete p;
            mtx_printlock.unlock();
            break;
        }
        p->printRecv(cwnd, ssthresh);
        mtx_printlock.unlock();
        mtx_ackReceivedQueue.lock();
        queue->push(p);
        mtx_ackReceivedQueue.unlock();
        usleep(10);
    }
    return 0;
}

void* RetransmissionHandle(void* data){
    std::queue<Packet*>* outgoingQueue = ((queue_cordination_packet*)data)->outgoingQueue;
    std::queue<Packet*>* incomingAcks = ((queue_cordination_packet*)data)->incomingAcks;
    std::unordered_map<unsigned short, Packet*>* inFlight = ((queue_cordination_packet*)data)->inflight;
    int counter = 0;
    while(!finished_transmission){
        // Await to process more ACKs
        usleep(100);
        counter += 100;
        // Go through the received queue and try to match with sent packets
        mtx_ackReceivedQueue.lock();
        bool ack_received = false;
        std::unordered_set<unsigned short> ackedPacketNumbers;
        std::unordered_set<unsigned short> highestAcks;
        while(!incomingAcks->empty()){
            Packet* front = incomingAcks->front();
            ackedPacketNumbers.insert(front->getAckNumber());
            highestAcks.insert(front->getSequenceNumber());
            delete front;
            incomingAcks->pop();
            ack_received = true;
        }
        mtx_ackReceivedQueue.unlock();
        
        // Retransmit packets if no ACK was received in the specified amount of time
        if(ack_received){
            counter = 0;
            int acks_gotten = 0;
            // Generate the highest ACK so-far received
            unsigned short curr_ack = ackNumber;
            for(unsigned short i = curr_ack; i != (curr_ack-1)%MAX_SEQ; i = (i + 1)%MAX_SEQ){
                if(highestAcks.find(i) != highestAcks.end()){
                    ackNumber = i;
                }
            }
            mtx_inflight.lock();
            for(auto it = ackedPacketNumbers.begin(); it != ackedPacketNumbers.end(); it++){
                unsigned short i = *it;
                std::unordered_map<unsigned short, Packet*>::iterator foundVal;
                while(inFlight->end() != (foundVal = inFlight->find(i))){
                    i = i > foundVal->second->getPayloadSize() ? i - foundVal->second->getPayloadSize() : MAX_SEQ + i - foundVal->second->getPayloadSize();
                    delete foundVal->second;
                    inFlight->erase(foundVal);
                    acks_gotten++;
                }
            }
            // LOGIC FOR INCREASING THE WINDOW SIZE!
            for(int i = 0; i<acks_gotten; i++){
                if(cwnd < ssthresh){
                    cwnd += 512;
                }else{
                    cwnd += 512*512 / cwnd;
                }
            }
            if(cwnd > MAX_CWND){
                cwnd = MAX_CWND;
            }
            mtx_inflight.unlock();
        }else if(counter > TIMEOUT){
            mtx_outgoingQueue.lock();
            mtx_inflight.lock();
            for(auto it = inFlight->begin(); it != inFlight->end(); it++){
                outgoingQueue->push(it->second);
            }
            
            // Packet Loss Event
            ssthresh = std::max(cwnd/2, 512*2);
            cwnd = 512;
            
            mtx_outgoingQueue.unlock();
            mtx_inflight.unlock();
            counter = 0;
        }
    }
    return 0;
}

void queue_packet(Packet* pack, std::queue<Packet*>* transmission_queue, std::unordered_map<unsigned short, Packet*>* inflight){
    mtx_outgoingQueue.lock();
    mtx_inflight.lock();
    transmission_queue->push(pack);
    inflight->insert(std::pair<unsigned short, Packet*>((pack->getSequenceNumber()+pack->getPayloadSize())%MAX_SEQ, pack));
    mtx_outgoingQueue.unlock();
    mtx_inflight.unlock();
}

Packet* awaitIncomingAck(std::queue<Packet*>* queue){
    mtx_ackReceivedQueue.lock();
    if(!queue->empty()){
        Packet* p = queue->front();
        queue->pop();
        mtx_ackReceivedQueue.unlock();
        return p;
    }
    mtx_ackReceivedQueue.unlock();
    usleep(TIMEOUT);
    return 0;
}

Packet* clearFromInFlight(std::unordered_map<unsigned short, Packet*>* inflight, Packet* packet){
    mtx_inflight.lock();
    auto it = inflight->find(packet->getSequenceNumber());
    if(it == inflight->end()){
        mtx_inflight.unlock();
        return 0;
    }
    Packet* p = it->second;
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
    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd < 0) {
        std::cerr<<"Could not create socket"<<std::endl;
        exit(2);
    }

    // Set the server address
    struct sockaddr_in serveraddr;
    std::memset(&serveraddr, 0, sizeof(serveraddr));

    // Set all of the server information
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    
    // Set localhost IP number
    if(std::strcmp(hostname, "localhost") == 0)
        std::strcpy(hostname, "127.0.0.1");

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
    std::unordered_map<unsigned short, Packet*>* inFlight = new std::unordered_map<unsigned short, Packet*>();
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
    Packet* ack;
    while(1){
        ack = awaitIncomingAck(received_acks);
        if(ack)
            break;
    }
    clearFromInFlight(inFlight, syn);
    
    
    // Check if the server accepted or rejected the connection
    if(!ack->SYNbit() || !ack->getAckNumber() || !ack->ACKbit()){
        std::cerr<<"Server rejected the connection :/"<<std::endl;
        exit(3);
    }
    delete syn;
    
    // Open the file to be tranmitted to the server!
    fd = open(filename, O_RDONLY);
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
    while(1){
        ack = awaitIncomingAck(received_acks);
        if(ack)
            break;
    }
    clearFromInFlight(inFlight, initial_data);

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
    ackNumber = ack->getSequenceNumber();
    sequence_number = ack->getAckNumber();
    bool finished_reading = false;
    while (1) {
        mtx_inflight.lock();
        int inFlightCount = 512 * (int)inFlight->size();
        mtx_inflight.unlock();
        for(; inFlightCount<cwnd; inFlightCount += 512 ){
            Packet* p = new Packet(sequence_number, ackNumber, 0,0,0);
            int incount = p->loadData(fd);
            queue_packet(p, to_send, inFlight);
            // Done reading in the file
            if(incount < 512){
                finished_reading = true;
                last_expected_ack = p->getSequenceNumber()+p->getPayloadSize();
                break;
            }
            sequence_number = (sequence_number + 512) % MAX_SEQ;
        }
        if(finished_reading){
            break;
        }
        usleep(10);
    }
    while(1){
        mtx_inflight.lock();
        if(inFlight->empty()){
            mtx_inflight.unlock();
            break;
        }
        mtx_inflight.unlock();
        usleep(10);
    }

    // Teardown
    Packet* finpacket = new Packet(0,0,0,0,1);
     finished_transmission = true;
    // Join threads
    pthread_join(queue_cordination,0);
    pthread_join(send_thread, 0);

    
    finpacket->sendPacket(socketfd);
    
    mtx_printlock.lock();
    finpacket->printSend(0, 0);
    mtx_printlock.unlock();
    usleep(2000000);
    pthread_join(receive_thread, 0);

    close(fd);
    close(socketfd);
}
