#include "HttpRequest.h"
#include "utils.h"
#include <vector>
#include <unistd.h>
#include <iostream>

// Defining protocols for HTTP
#define GET 0
#define POST 1
#define UNKNOWN 2

using namespace std;

HttpRequest::HttpRequest(int stream_fd){
    char b;
    string buffer;
    // First line, eg GET / HTTP/1.1
    while(read(stream_fd, &b, 1) && b != '\n')
        buffer.push_back(b);
    vector<string> request_line = split(buffer, " ");
    m_url = request_line[1];
    m_data += buffer;

    buffer = "";
    while(read(stream_fd, &b, 1) && b != '\n')
        buffer.push_back(b);
    cout<<buffer<<endl;
    m_data += buffer;
}



std::string HttpRequest::get_data(){
    return m_data;
}
std::string HttpRequest::get_url(){
    return m_url;
}
