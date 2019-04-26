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
    // Set the status of the response based on if the file exists
    m_status = m_file->file_exists() ? 200 : 404;
    // Create a new string stream
    m_stringstream = new ostringstream();
}

HttpResponse::~HttpResponse(){
    // Clean up the string stream
    delete(m_stringstream);
}

int HttpResponse::flush_and_close(){
    // Create the header in the right format
    format_header(m_stringstream);
    // Output response to console during dev
    // cout<<m_stringstream->str()<<endl;
    // Write the header to the output stream
    write(m_ostream, m_stringstream->str().c_str(), m_stringstream->str().size());

    // Open the output stream with write permissions
    FILE* fp = fdopen(m_ostream, "w");
    // Get the file stream from the file module
    ifstream* infile = m_file->get_file_stream();
    char c;
    // Write the file one byte at a time to the browser
    while (infile->get(c)){
        putc(c, fp);
    }
    // Clean up before returning
    fflush(fp);
    fclose(fp);
    return 0;
}
void HttpResponse::format_header(std::ostringstream* ss){
    // Get the current time for the HTTP Response header
    chrono::system_clock::time_point m_now = chrono::system_clock::now();
    time_t m_time = chrono::system_clock::to_time_t(m_now);
    struct tm* m_curtime = gmtime(&m_time);
    // Format the HTTP Response header that is sent to the browser
    *ss << "HTTP/1.1 "<< m_status << " " << get_status_message(m_status) << endl;
    *ss << "Connection: close"<<endl;
    *ss << "Date: "<<put_time(m_curtime, "%a, %e %b %G %T GMT")<<endl;
    *ss << "Server: Apache/2.2.3 (Ubuntu)" <<endl;
    *ss << "Last-Modified: "<<put_time(m_file->get_mod_time(), "%a, %e %b %G %T GMT")<<endl;
    *ss << "Content-Length: "<<m_file->file_size()<<endl;
    *ss << "Content-Type: "<< get_content_type(m_file->get_filename()) << endl <<endl;
}
