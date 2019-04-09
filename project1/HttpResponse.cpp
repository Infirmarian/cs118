#include "stdio.h"

#include "HttpResponse.h"

HttpResponse::HttpResponse(int status, int out_fd){
    m_status = status;
    m_ostream = out_fd;
}

HttpResponse::flush_and_close(){

    close(m_ostream);
}

std::string HttpResponse::format_header(){
    string header = "";
    ostringstream ss;
    ss << "HTTP/1.1 "<<m_status<< " "<<get_status_message(m_status)<<endl;
    ss << "Connection: close"<<endl;
    ss << "Date: Mon, 08 Apr 2019 15:44:04 GMT" <<endl;
    ss.str();
}

int HttpResponse::set_data_from_file(std::string filename){

}
