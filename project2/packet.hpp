//
//  packet.cpp
//  project2
//
//  Created by Robert Geil on 5/3/19.
//  Copyright © 2019 Robert Geil, Levi Weible. All rights reserved.
//

#ifndef header_hpp
#define header_hpp
typedef unsigned char byte;
#define ACK 0x80
#define SYN 0x40
#define FIN 0x20
#define MAX_SEQ 25600
#define HEAD_LENGTH 12
#define DATA_LENGTH 512
class Packet{
public:
	Packet(short sequenceNumber, bool ack, bool syn, bool fin);
	Packet(byte* data, short length);
	Packet(int socket);
    short getSequenceNumber();
    short getAckNumber();
	bool ACKbit();
	bool SYNbit();
	bool FINbit();
	short getPayloadSize();
	int sendPacket(int socket);
private:
    // FORMAT OF header
    /*  [0-1] - sequence number
        [2-3] - ack number
        [4]   - ACK, SYN and FIN flags (in the most significant positions)
	 	[5-6] - Payload size (bytes)
	 	[7-11]- padding chars
	 */
    byte m_header[HEAD_LENGTH];
	byte m_data[DATA_LENGTH];
};

#endif /* header_hpp */
