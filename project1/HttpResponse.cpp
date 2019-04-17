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

int HttpResponse::flush_and_close(){
    string header = format_header();
    cout<<header<<endl;
    write(m_ostream, header.c_str(), header.size());
    FILE* fp = fdopen(m_ostream, "w");
    ifstream* infile = m_file->get_file_stream();
    char c;
    while (infile->get(c)){
        putc(c, fp);
    }
    fflush(fp);
    fclose(fp);
    return 0;
}
std::string HttpResponse::format_header(){
    ostringstream ss;
    ss << "HTTP/1.1 "<< m_status << " " << get_status_message(m_status) << endl;
    ss << "Connection: close"<<endl;
    ss << "Date: Mon, 08 Apr 2019 15:44:04 GMT"<<endl; //TODO: Get the actual date
    ss << "Server: Apache/2.2.3 (Ubuntu)" <<endl;
    ss << "Last-Modified: Tue, 09 Aug 2011 15:11:03 GMT"<<endl; //TODO: Get the file last modify time
    ss << "Content-Length: "<<m_file->file_size()<<endl;
    ss << "Content-Type: "<< get_content_type(m_file->get_filename()) << endl <<endl;
    return ss.str();
}
