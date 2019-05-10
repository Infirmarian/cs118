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
}

// Constructor to build a packet based on received data
Packet::Packet(byte* data, short length){
    // Setup single data buffer
    m_raw_data = new byte[HEAD_LENGTH + DATA_LENGTH];
    m_header = m_raw_data;
    m_data = m_raw_data + HEAD_LENGTH;
    
    if(length < HEAD_LENGTH){
        std::cerr<<"Packet is too short to contain a header"<<std::endl;
        return;
    }
    memcpy(m_header, data, HEAD_LENGTH);
    memcpy(m_data, data + HEAD_LENGTH, length - HEAD_LENGTH);
}

Packet::Packet(int socket){
    // Setup single data buffer
    m_raw_data = new byte[HEAD_LENGTH + DATA_LENGTH];
    m_header = m_raw_data;
    m_data = m_raw_data + HEAD_LENGTH;
    
    recv(socket, m_raw_data, HEAD_LENGTH + DATA_LENGTH, 0);
}

Packet::Packet(int socket, struct sockaddr* addr, socklen_t* len){
    // Setup single data buffer
    m_raw_data = new byte[HEAD_LENGTH + DATA_LENGTH];
    m_header = m_raw_data;
    m_data = m_raw_data + HEAD_LENGTH;
    
    recvfrom(socket, m_raw_data, HEAD_LENGTH+DATA_LENGTH, MSG_WAITALL, addr, len);
}

// Destructor makes sure to free data members
Packet::~Packet(){
    delete [] m_raw_data;
}


// This function gets the sequence number from a header by
// shifting and ORing the first two bytes in the m_data array
short Packet::getSequenceNumber(){
    short data = (short)(m_header[0] << 8 | m_header[1]);
    return data % MAX_SEQ;
}

// This function gets the acknowledgement number from a header
// by shifting and ORing the 3rd and 4th bytes of m_data
short Packet::getAckNumber(){
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

short Packet::getPayloadSize(){
    short size = (short)(m_header[5] << 8 | m_header[6]);
    return size % 512;
}

byte* Packet::getData(){
    return m_data;
}

// This function uses the "send" function to transport the packet (header and all) over the specified socket
// Return: 0 indicates successful tranmission of the packet, -1 indicates error, and errno is set
int Packet::sendPacket(int socket){
    return (int) send(socket, m_raw_data, HEAD_LENGTH + this->getPayloadSize(), 0);
}
int Packet::sendPacket(int socket, struct sockaddr * addr, socklen_t len){
    return (int) sendto(socket, m_raw_data, HEAD_LENGTH + this->getPayloadSize(), 0, addr, len);
}


// This function pretty prints the packet information for debugging/testing
void Packet::toString(){
    std::cout<<"SEQ Number: "<<getSequenceNumber()<<"\nACK Number: "<<getAckNumber()<<std::endl;
    std::cout<<"ACK: "<<ACKbit()<<"\nSYN: "<<SYNbit()<<"\nFIN: "<<FINbit()<<std::endl;
    std::cout<<"Length: "<<getPayloadSize()<<std::endl;
    std::cout<<"------------------------------------------\n"<<getData()<<std::endl;
}
