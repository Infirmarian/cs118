//
//  header.cpp
//  project2
//
//  Created by Robert Geil on 5/3/19.
//  Copyright © 2019 Robert Geil. All rights reserved.
//

#include "header.hpp"
#define MAX_SEQ 25600

// This function gets the sequence number from a header by
// shifting and ORing the first two bytes in the m_data array
short Header::getSequenceNumber(){
    short data = (short)(m_data[0] << 8 | m_data[1]);
    return data % MAX_SEQ;
}

// This function gets the acknowledgement number from a header
// by shifting and ORing the 3rd and 4th bytes of m_data
short Header::getAckNumber(){
    short data = (short)(m_data[2] << 8 | m_data[3]);
    return data % MAX_SEQ;
}

// The following 3 functions extract the requested bit flag from
// the 5th byte in the m_data array and return the result as a boolean
bool Header::ACKbit(){
    bool ack = m_data[4] & ACK;
    return ack;
}

bool Header::SYNbit(){
    bool syn = m_data[4] & SYN;
    return syn;
}

bool Header::FINbit(){
    bool fin = m_data[4] & FIN;
    return fin;
}

