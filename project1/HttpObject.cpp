#include "HttpObject.h"
#include <vector>
#include <unistd.h>
#include <iostream>

// Defining protocols for HTTP
#define GET 0
#define POST 1
#define UNKNOWN 2

using namespace std;

HttpObject::HttpObject(int stream_fd){
    char b;
    string protocol;
    while(read(stream_fd, &b, 1) && b != '\n')
        protocol.push_back(b);
    int start = protocol.find(" ");
    int end = protocol.find(" HTTP");
    m_url = m_data.substr(m_data.find(" "), m_data.find(" HTTP"));

}



std::string HttpObject::get_data(){
    return m_data;
}
std::string HttpObject::get_url(){
    return m_url;
}
