#include "HttpRequest.h"
#include "utils.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <stdio.h>

using namespace std;

HttpRequest::HttpRequest(int stream_fd){
    // Create a new string stream on the heap for the HTTP Request message
    string* ss = new string();
    char inchar = 0;
    // Get the filestream
    FILE* fp = fdopen(stream_fd, "r");
    char wrap_buffer[4] = {0,0,0,0};
    int counter = 0;
    inchar = getc(fp);
    // Get the message from the filestream and place it in the string stream
    while(inchar != EOF){
        ss->push_back(inchar);
        if(inchar == '\n' && wrap_buffer[(counter-1)%4] == '\r' && wrap_buffer[(counter-2)%4] == '\n'
                && wrap_buffer[(counter-3)%4] == '\r')
            break;
        wrap_buffer[counter%4] = inchar;
        counter ++;
        inchar = getc(fp);
    }
    // Use the utils split function to format the message on new lines
    vector<string>* request_line = split(*ss, " ");
    // Set the URL
    m_url = (*request_line)[1];
    // Output the string stream of the request to the console
    cout<<*ss<<flush;
    // Clean up
    delete(request_line);
    delete(ss);
}

HttpRequest::~HttpRequest(){
    
}


std::string HttpRequest::get_data(){
    // Return the data of the request to the caller
    return m_data;
}
std::string HttpRequest::get_url(){
    // Return the request URL to the caller
    return m_url;
}
