#include <vector>
#include <string>

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H
class HttpResponse{
public:
    HttpResponse(int status, int out_fd);
    ~HttpResponse();
    std::string get_url();
    int flush_and_close();
    int set_data_from_file(std::string filename);
private:
    int m_status;
    int m_ostream;
    std::string format_header();
};

#endif