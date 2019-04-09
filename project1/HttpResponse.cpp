#include <stdio.h>
#include <sstream> 
#include <unistd.h>

#include "utils.h"
#include "HttpResponse.h"

using namespace std;

HttpResponse::HttpResponse(int status, int out_fd, int file_fd, std::string content_type){
    m_status = status;
    m_ostream = out_fd;
    m_data_type = content_type;
    m_ifstream = file_fd;
}

int HttpResponse::flush_and_close(){
    close(m_ostream);
    return 0;
}

std::string HttpResponse::format_header(){
    string header = "";
    ostringstream ss;
    ss << "HTTP/1.1 "<<m_status<< " "<<get_status_message(m_status)<<endl;
    ss << "Connection: close"<<endl;
    ss << "Date: Mon, 08 Apr 2019 15:44:04 GMT"<<endl;
    ss << "Server: Apache/2.2.3 (Ubuntu)" <<endl;
    ss << "Last-Modified: "<<endl;
    ss << "Content-Length: 17"<<endl;
    ss << "Content-Type: "<< m_data_type << endl;
    return ss.str();
}