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
    // Close the socket when interrupted
    close(sock);
    _exit(sig);
}

int main(int argc, char** argv){
    // Make sure that the number of arguments is correct
    if(argc != 3){
        cerr<<"Incorrect number of arguments. Expected 3, got "<<argc<<endl;
        cerr<<"Correct usage: ./server [address] [port]"<<endl;
        exit(1);
    }

    // Set the errors if an interrupt occurs
    if(signal(SIGINT, close_socket) == SIG_ERR){
        cerr<<"Unable to set signal handler to properly close socket: "<<strerror(errno)<<endl;
    }

    if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
        cerr<<"Unable to set signal handler to ignore closed pipes: "<<strerror(errno)<<endl;
    }

    string host = argv[1];
    int port = atoi(argv[2]);

    // Get the mapping of all files in the current directory
    unordered_map<string, string> filemap;
    load_filemap(filemap);

    // Attempt to open a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    // Make sure the socket opened correctly
    if(sock == -1){
        cerr<<"Unable to open a socket: "<<strerror(errno)<<endl;
        exit(2);
    }
    // Set the address and port number for the socket
    sockaddr_in address;
    address.sin_addr.s_addr = inet_addr(host.c_str());
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    // Attempt to bind the socket
    // Output error and exit if unable to bind
    if(::bind(sock, (const sockaddr* ) &address, sizeof(address)) != 0){
        cerr << "Unable to bind address to socket: "<<strerror(errno)<<endl;
        exit(2);
    }
    // Attempt to listen on the specified port
    // Output error and exit if unable to listen
    if(listen(sock, 10)){
        cerr<<"Unable to listen on port "<<port<<": "<<strerror(errno)<<endl;
        exit(2);
    }
    // Output to console which port the server is listening on
    cout<<"Listening on port "<<port<<endl;
    // Define socket
    sockaddr_un incoming;
    socklen_t size = sizeof(struct sockaddr_un);
    // Define an iterator for the file map
    unordered_map<string, string>::iterator it;
    // Loop until servre is closed listening on the specified port
    while(1){
        // Accept a request when given one
        int instream = accept(sock, (struct sockaddr *) &incoming, &size);
        // Create a new HTTP Request
        HttpRequest* h = new HttpRequest(instream);

        // Check to see if the filename requested is valid 
        it = filemap.find(convert_url_to_file(h->get_url()));
        // Default filename is "" for a file that is not valid
        string filename = "";
        // If filename is valid, set the filename to the actual filename
        if(it != filemap.end())
            filename = it->second;

        // Create a new output file
        File* out_file = new File(filename);
        // Create a new HTTP Response
        HttpResponse* r = new HttpResponse(instream, out_file);
        
        // Format the HTTP Response and send it to the browser
        r->flush_and_close();

        // Cleanup when done
        delete(h);
        delete(out_file);
        delete(r);
    }
}