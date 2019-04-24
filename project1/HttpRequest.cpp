#include "HttpRequest.h"
#include "utils.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <stdio.h>

using namespace std;

HttpRequest::HttpRequest(int stream_fd){
    string* ss = new string();
    char inchar = 0;
    FILE* fp = fdopen(stream_fd, "r"); // get filestream
    char wrap_buffer[4] = {0,0,0,0};
    int counter = 0;
    inchar = getc(fp);
    // First line, eg GET / HTTP/1.1
    while(inchar != EOF){
        ss->push_back(inchar);
        if(inchar == '\n' && wrap_buffer[(counter-1)%4] == '\r' && wrap_buffer[(counter-2)%4] == '\n'
                && wrap_buffer[(counter-3)%4] == '\r')
            break;
        wrap_buffer[counter%4] = inchar;
        counter ++;
        inchar = getc(fp);
    }
    vector<string>* request_line = split(*ss, " ");
    m_url = (*request_line)[1];
    cout<<*ss<<flush;
    delete(request_line);
    delete(ss);
}

HttpRequest::~HttpRequest(){
    
}


std::string HttpRequest::get_data(){
    return m_data;
}
std::string HttpRequest::get_url(){
    return m_url;
}
