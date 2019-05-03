//junk.cpp
// This file generates an arbitrary number of random bytes
// into a specified file. Useful for generating large files
// for transmission across the server
#include <stdlib.h>
#include <time.h>
#include <fstream>
#include <iostream>

using namespace std;

int main(int argc, char** argv){
    if(argc != 3){
        cerr <<"Enter arguments in the form ./junk [filename] [bytes]"<<endl;
        exit(1);
    }
    ofstream output;
    output.open(argv[1], ios::binary);
    long long filelength = atoll(argv[2]);
    cout<<"File length: "<<filelength<<endl;
    char byte;
    srand(time(0));
    for(long long i = 0; i<filelength; i++){
        byte = rand()%sizeof(char);
        output << byte;
    }
    output.flush();
    output.close();
}