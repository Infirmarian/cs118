#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <vector>


#include "utils.h"
#include "HttpObject.h"
using namespace std;

int main(int argc, char** argv){
    if(argc != 3){
        cerr<<"Incorrect number of arguments. Expected 3, got "<<argc<<endl<<flush;
        exit(1);
    }

    string host = argv[1];
    int port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1){
        cerr<<"Unable to open a socket: "<<strerror(errno)<<endl;
        exit(2);
    }
    sockaddr_in address;
    address.sin_addr.s_addr = inet_addr(host.c_str());
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    bind(sock, (const sockaddr* ) &address, sizeof(address));
    if(listen(sock, 5)){
        cerr<<"Unable to listen on port "<<port<<": "<<strerror(errno)<<endl;
        exit(2);
    }
    cout<<"Listening on port "<<port<<endl;
    sockaddr_un incoming;
    socklen_t size = sizeof(struct sockaddr_un);
    while(1){
        int instream = accept(sock, (struct sockaddr *) &incoming, &size);
        HttpObject* h = new HttpObject(instream);
        string data = h->get_data();
        cout<<data<<endl;
    }

}