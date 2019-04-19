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
#include <fcntl.h>
#include <unordered_map>
#include <signal.h>
#include <sys/socket.h>

#include "utils.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "File.h"

using namespace std;
static int sock;

void close_socket(int sig){
    close(sock);
    _exit(sig);
}

int main(int argc, char** argv){
    if(argc != 3){
        cerr<<"Incorrect number of arguments. Expected 3, got "<<argc<<endl;
        cerr<<"Correct usage: ./server [address] [port]"<<endl;
        exit(1);
    }

    if(signal(SIGINT, SIG_IGN) == SIG_ERR){
        cerr<<"Unable to set signal handler to properly close socket: "<<strerror(errno)<<endl;
    }

    if(signal(SIGPIPE, close_socket) == SIG_ERR){
        cerr<<"Unable to set signal handler to ignore closed pipes: "<<strerror(errno)<<endl;
    }

    string host = argv[1];
    int port = atoi(argv[2]);

    // Get the mapping of all files in the current directory
    unordered_map<string, string> filemap;
    load_filemap(filemap);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1){
        cerr<<"Unable to open a socket: "<<strerror(errno)<<endl;
        exit(2);
    }
    sockaddr_in address;
    address.sin_addr.s_addr = inet_addr(host.c_str());
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if(::bind(sock, (const sockaddr* ) &address, sizeof(address)) != 0){
        cerr << "Unable to bind address to socket: "<<strerror(errno)<<endl;
        exit(2);
    }
    if(listen(sock, 10)){
        cerr<<"Unable to listen on port "<<port<<": "<<strerror(errno)<<endl;
        exit(2);
    }
    cout<<"Listening on port "<<port<<endl;
    sockaddr_un incoming;
    socklen_t size = sizeof(struct sockaddr_un);
    //TODO: Make this section threaded to allow multiple connections
    //TODO: figure out why this sometimes seg faults
    unordered_map<string, string>::iterator it;
    while(1){
        int instream = accept(sock, (struct sockaddr *) &incoming, &size);
        HttpRequest* h = new HttpRequest(instream);

        it = filemap.find(convert_url_to_file(h->get_url()));
        string filename = "";
        if(it != filemap.end())
            filename = it->second;

        File* out_file = new File(filename);
        HttpResponse* r = new HttpResponse(instream, out_file);
        
        r->flush_and_close();
        //close(instream);

        delete(h);
        delete(out_file);
        delete(r);
    }
}