//
//  server.cpp
//  project2
//
//  Created by Robert Geil on 5/3/19.
//  Copyright © 2019 Robert Geil. All rights reserved.
//

#include <iostream>

int main(int argc, char** argv){
    if(argc != 2){
        std::cerr<<"Bad arguments, expected \n./server [PORT]"<<std::endl;
        exit(1);
    }
    int port = atoi(argv[1]);
    if(port <= 0){
        std::cerr<<"Bad port number "<<port<<" provided"<<std::endl;
        exit(1);
    }
	std::cout<<"Hello client!"<<std::endl;

}
