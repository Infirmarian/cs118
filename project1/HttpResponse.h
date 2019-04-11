#include <vector>
#include <string>

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H
class HttpResponse{
public:
    HttpResponse(int status, int out_fd, int file_fd, std::string content_type);
    ~HttpResponse();
    std::string get_url();
    int flush_and_close();
private:
    int m_status;
    int m_ostream;
    int m_ifstream;
    std::string m_data_type;
    std::string format_header();
};

#endif