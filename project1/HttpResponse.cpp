#include <stdio.h>
#include <sstream> 
#include <fstream>
#include <unistd.h>
#include <iostream>


#include "utils.h"
#include "HttpResponse.h"

using namespace std;

HttpResponse::HttpResponse(int out_fd, File* file){
    m_ostream = out_fd;
    m_file = file;
    m_status = m_file->file_exists() ? 200 : 404;
}
HttpResponse::~HttpResponse(){
    
}

int HttpResponse::flush_and_close(bool close_connection){
    string header = format_header(close_connection);
    write(m_ostream, header.c_str(), header.size());
    if(m_file->file_exists()){
        char b;
        //TODO: Buffer this output
        while(m_file->get_file_stream()->read(&b, 1)){
            write(m_ostream, &b, 1);
        }
    }else{
        write(m_ostream, "<!DOCTYPE html><html><body><h1>Error 404</h1><p>The page you are looking for doesn't exist</p></body></html>", 109);
    }
    if(close_connection){
        close(m_ostream);
        delete this;
    }
    return 0;
}
std::string HttpResponse::format_header(bool close_connection){
    ostringstream ss;
    ss << "HTTP/1.1 "<< m_status << " " << get_status_message(m_status) << endl;
    ss << "Connection: "<<(close_connection ? "Close" : "timeout=10")<<endl;
    ss << "Date: Mon, 08 Apr 2019 15:44:04 GMT"<<endl; //TODO: Get the actual date
    ss << "Server: Apache/2.2.3 (Ubuntu)" <<endl;
    ss << "Last-Modified: Tue, 09 Aug 2011 15:11:03 GMT"<<endl; //TODO: Get the file last modify time
    if(m_file->file_exists()){
        ss << "Content-Length: "<<m_file->file_size()<<endl;
        ss << "Content-Type: "<< get_content_type(m_file->get_filename()) << endl <<endl;
    }else{
       ss << "Content-Length: "<<109<<endl;
        ss << "Content-Type: text/html" << endl <<endl;
    }
    return ss.str();
}