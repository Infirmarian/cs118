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
	Packet(short sequenceNumber, short ackNumber, bool ack, bool syn, bool fin);
	Packet(byte* data, unsigned short length);
	Packet(int socket, bool fin);
	Packet(int socket, struct sockaddr* addr, socklen_t* len);
	~Packet();
    unsigned short getSequenceNumber() const;
    unsigned short getAckNumber();
	bool ACKbit();
	bool SYNbit();
	bool FINbit();
	unsigned short getPayloadSize();
	byte* getData();
	int sendPacket(int socket);
	int sendPacket(int socket, struct sockaddr* addr, socklen_t len);
	void toString();
	short loadData(int fd);
	void printSend(int cwnd, int ssthresh);
	void printRecv(int cwnd, int ssthresh);
	bool timeoutHit();
	bool operator<(const Packet& other) const;
	long long getCreationTime();
	void setDuplicate();
	short getExpectedAckNumber();
	void setHasBeenAcked();
	bool getHasBeenAcked();
	
private:
    // FORMAT OF header
    /*  [0-1] - sequence number
        [2-3] - ack number
        [4]   - ACK, SYN and FIN flags (in the most significant positions)
	 	[5-6] - Payload size (bytes)
	 	[7-11]- padding chars
	 */
	byte* m_raw_data;
	byte* m_data;
	byte* m_header;
	bool m_timeout;
	bool m_duplicate;
	bool m_hasBeenAcked;
};

#endif /* header_hpp */
