//
//  packet.cpp
//  project2
//
//  Created by Robert Geil on 5/3/19.
//  Copyright © 2019 Robert Geil, Levi Weible. All rights reserved.
//
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <chrono>
#include <cerrno>

#include "packet.hpp"

// Constructor to set sequence number, ack, syn and fin bit
Packet::Packet(short sequenceNumber, short ackNumber, bool ack, bool syn, bool fin){
    // Setup single data buffer
    m_raw_data = new byte[HEAD_LENGTH + DATA_LENGTH];
    m_header = m_raw_data;
    m_data = m_raw_data + HEAD_LENGTH;
    
    // Set all bytes of data and header to zero initially
    std::memset(m_header, 0, 12);
    std::memset(m_data, 0, 512);
    sequenceNumber = sequenceNumber % MAX_SEQ; // Limit sequence number
    ackNumber = ackNumber % MAX_SEQ;
    
    // Set ACK and SEQ numbers
    m_header[0] = (sequenceNumber >> 8) & 0xff;
    m_header[1] = sequenceNumber & 0xff;
    m_header[2] = (ackNumber >> 8) & 0xff;
    m_header[3] = ackNumber & 0xff;
    
    
    byte flags = 0;
    if(ack)
        flags = flags | ACK;
    if(syn)
        flags = flags | SYN;
    if(fin)
        flags = flags | FIN;
    m_header[4] = flags;
    m_header[5] = m_header[6] = 0;
    m_timeout = false;
    m_duplicate = false;

}

// Constructor to build a packet based on received data
Packet::Packet(byte* data, unsigned short length){
    // Setup single data buffer
    m_raw_data = new byte[HEAD_LENGTH + DATA_LENGTH];
    m_header = m_raw_data;
    m_data = m_raw_data + HEAD_LENGTH;
    
    if(length < HEAD_LENGTH){
        std::cerr<<"Packet is too short to contain a header"<<std::endl;
        return;
    }
    if(length > DATA_LENGTH + HEAD_LENGTH){
        std::cerr<<"Packet is WAY too long!"<<std::endl;
        assert(false);
    }
    memcpy(m_header, data, HEAD_LENGTH);
    memcpy(m_data, data + HEAD_LENGTH, length - HEAD_LENGTH);
    m_timeout = false;

    m_duplicate = false;
}

Packet::Packet(int socket){
    // Setup single data buffer
    m_raw_data = new byte[HEAD_LENGTH + DATA_LENGTH];
    m_header = m_raw_data;
    m_data = m_raw_data + HEAD_LENGTH;

    // Setup timout interval
    struct timeval timeout={10,0};
    setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

    int rec_check = recv(socket, m_raw_data, HEAD_LENGTH + DATA_LENGTH, 0);

    if (rec_check < 0) {
        std::cerr<<"Timout interval hit"<<std::endl;
        m_timeout = true;
    }else{
        m_timeout = false;
    }
    m_duplicate = false;
}

Packet::Packet(int socket, struct sockaddr* addr, socklen_t* len){
    // Setup single data buffer
    m_raw_data = new byte[HEAD_LENGTH + DATA_LENGTH];
    m_header = m_raw_data;
    m_data = m_raw_data + HEAD_LENGTH;
    
    // Setup timout interval
    struct timeval timeout={10,0};
    setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(struct timeval));

    int rec_check = recvfrom(socket, m_raw_data, HEAD_LENGTH+DATA_LENGTH, MSG_WAITALL, addr, len);

    if (rec_check < 0 && errno == ETIMEDOUT) {
        std::cerr<<"Timout interval hit"<<std::endl;
        m_timeout = true;
    }else if(rec_check < 0){
        std::cerr<<"Error with reading from socket: "<<strerror(errno)<<std::endl;
        assert(false);
    }else{
        m_timeout = false;
    }
    m_duplicate = false;
    m_hasBeenAcked = false;

}

// Destructor makes sure to free data members
Packet::~Packet(){
    delete [] m_raw_data;
}

bool Packet::operator<(const Packet &other) const{
    return this->getSequenceNumber() < other.getSequenceNumber();
}


// This function gets the sequence number from a header by
// shifting and ORing the first two bytes in the m_data array
unsigned short Packet::getSequenceNumber() const{
    short data = (short)(m_header[0] << 8 | m_header[1]);
    return data % MAX_SEQ;
}

// This function gets the acknowledgement number from a header
// by shifting and ORing the 3rd and 4th bytes of m_data
unsigned short Packet::getAckNumber(){
    short data = (short)(m_header[2] << 8 | m_header[3]);
    return data % MAX_SEQ;
}

// The following 3 functions extract the requested bit flag from
// the 5th byte in the m_data array and return the result as a boolean
bool Packet::ACKbit(){
    bool ack = m_header[4] & ACK;
    return ack;
}

bool Packet::SYNbit(){
    bool syn = m_header[4] & SYN;
    return syn;
}

bool Packet::FINbit(){
    bool fin = m_header[4] & FIN;
    return fin;
}

unsigned short Packet::getPayloadSize(){
    short size = (short)(m_header[5] << 8 | m_header[6]);
    return size;
}

byte* Packet::getData(){
    return m_data;
}

// This function uses the "send" function to transport the packet (header and all) over the specified socket
// Return: 0 indicates successful tranmission of the packet, -1 indicates error, and errno is set
int Packet::sendPacket(int socket){
    if(send(socket, m_raw_data, HEAD_LENGTH + this->getPayloadSize(), 0) == -1){
        std::cerr<<"Unable to send packet: "<<strerror(errno)<<std::endl;
        return -1;
    }
    return 0;
}
int Packet::sendPacket(int socket, struct sockaddr * addr, socklen_t len){
    return (int) sendto(socket, m_raw_data, HEAD_LENGTH + this->getPayloadSize(), 0, addr, len);
}

// Load as much data as needed into the specified file from the givin file descriptor
short Packet::loadData(int fd){
    short res = (short) read(fd, m_data, DATA_LENGTH);
    // Set the data length field of the packet based on how much data was read in
    m_header[5] = (res >> 8) & 0xff;
    m_header[6] = res & 0xff;
    return res;
}


// This function pretty prints the packet information for debugging/testing
void Packet::toString(){
    std::cout<<"SEQ Number: "<<getSequenceNumber()<<"\nACK Number: "<<getAckNumber()<<std::endl;
    std::cout<<"ACK: "<<ACKbit()<<"\nSYN: "<<SYNbit()<<"\nFIN: "<<FINbit()<<std::endl;
    std::cout<<"Length: "<<getPayloadSize()<<std::endl;
    std::cout<<"------------------------------------------\n"<<getData()<<std::endl;
}

// Print a packet message that was sent from caller
void Packet::printSend(int cwnd, int ssthresh){
    std::cout<<"SEND "<<getSequenceNumber()<<" "<<getAckNumber()<< " "<<cwnd<<" "<<ssthresh;
    if (ACKbit()) {
        std::cout<<" [ACK]";
    }
    if (SYNbit()) {
        std::cout<<" [SYN]";
    }
    if (FINbit()) {
        std::cout<<" [FIN]";
    }
    if (this->m_duplicate) {
        std::cout<<" [DUP]";
    }
    std::cout<<std::endl;
}

// Print a packet message that was received by caller
void Packet::printRecv(int cwnd, int ssthresh){
    std::cout<<"RECV "<<getSequenceNumber()<<" "<<getAckNumber()<< " "<<cwnd<<" "<<ssthresh;
    if (ACKbit()) {
        std::cout<<" [ACK]";
    }
    if (SYNbit()) {
        std::cout<<" [SYN]";
    }
    if (FINbit()) {
        std::cout<<" [FIN]";
    }
    std::cout<<std::endl;
}

bool Packet::timeoutHit() {
    return m_timeout;
}

void Packet::setDuplicate(){
    m_duplicate = true;
}

short Packet::getExpectedAckNumber(){
    return this->getSequenceNumber() + this->getPayloadSize();
}

bool Packet::getHasBeenAcked(){
    return m_hasBeenAcked;
}

void Packet::setHasBeenAcked(){
    m_hasBeenAcked = true;
}
