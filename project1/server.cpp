#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <iostream>
#include <stdlib.h>

#include "utils.h"
using namespace std;

int main(int argc, char** argv){
    if(argc != 3){
        cerr<<"Incorrect number of arguments. Expected 3, got "<<argc<<endl<<flush;
        exit(1);
    }
    string host = argv[1];
    string port = argv[2];

}