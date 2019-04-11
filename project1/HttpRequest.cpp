#include "HttpRequest.h"
#include "utils.h"
#include <vector>
#include <unistd.h>
#include <iostream>
#include <strstream>

// Defining protocols for HTTP
#define GET 0
#define POST 1
#define UNKNOWN 2

using namespace std;

HttpRequest::HttpRequest(int stream_fd){
    strstream ss;
    char buf[5];
    buf[4] = 0;
    // First line, eg GET / HTTP/1.1
    while(read(stream_fd, buf, 4)){
        ss << buf;
        if(buf[0] == '\r' && buf[1] == '\n' && buf[2] == '\r' && buf[3] == '\n')
            break;
    }
    vector<string> request_line = split(ss.str(), " ");
    m_url = request_line[1];

    cout<<ss.str()<<flush;
}

HttpRequest::~HttpRequest(){
    
}


std::string HttpRequest::get_data(){
    return m_data;
}
std::string HttpRequest::get_url(){
    return m_url;
}
