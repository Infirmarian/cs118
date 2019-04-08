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
    while(read(stream_fd, &b, 1)){
        if(b == '\n' && m_data[m_data.size()-1] == '\n')
            break;
        m_data.push_back(b);
    }
}



std::string HttpObject::get_data(){
    return m_data;
}
std::string HttpObject::get_url(){
    return m_url;
}
