#include <stdio.h>
#include <sstream> 
#include <fstream>
#include <unistd.h>
#include <iostream>
#include <ctime>
#include <chrono>
#include <iomanip>


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
    ostringstream* header = format_header();
    cout<<header->str()<<endl;
    write(m_ostream, header->str().c_str(), header->str().size());
    FILE* fp = fdopen(m_ostream, "w");
    ifstream* infile = m_file->get_file_stream();
    char c;
    while (infile->get(c)){
        putc(c, fp);
    }
    fflush(fp);
    fclose(fp);
    header->flush();
    delete(header);
    return 0;
}
std::ostringstream* HttpResponse::format_header(){
    ostringstream* ss = new ostringstream();
    chrono::system_clock::time_point m_now = chrono::system_clock::now();
    time_t m_time = chrono::system_clock::to_time_t(m_now);
    struct tm* m_curtime = gmtime(&m_time);
    *ss << "HTTP/1.1 "<< m_status << " " << get_status_message(m_status) << endl;
    *ss << "Connection: close"<<endl;
    *ss << "Date: "<<put_time(m_curtime, "%a, %e %b %G %T GMT")<<endl;
    *ss << "Server: Apache/2.2.3 (Ubuntu)" <<endl;
    *ss << "Last-Modified: "<<put_time(m_file->get_mod_time(), "%a, %e %b %G %T GMT")<<endl;
    *ss << "Content-Length: "<<m_file->file_size()<<endl;
    *ss << "Content-Type: "<< get_content_type(m_file->get_filename()) << endl <<endl;
    return ss;
}
