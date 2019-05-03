//
//  header.hpp
//  project2
//
//  Created by Robert Geil on 5/3/19.
//  Copyright © 2019 Robert Geil. All rights reserved.
//

#ifndef header_hpp
#define header_hpp
typedef unsigned char byte;
#define ACK 0x80
#define SYN 0x40
#define FIN 0x20
class Header{
public:
    short getSequenceNumber();
    short getAckNumber();
	bool ACKbit();
	bool SYNbit();
	bool FINbit();
private:
    // FORMAT OF header
    /*  [0-1] - sequence number
        [2-3] - ack number
        [4]   - ACK, SYN and FIN flags (in the most significant positions)
	 	[5-11]- padding chars
	 */
    byte m_data[12];
};

#endif /* header_hpp */
