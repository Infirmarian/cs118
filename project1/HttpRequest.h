#include <vector>
#include <string>

#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H
class HttpRequest{
public:
    HttpRequest(int stream_fd);
    HttpRequest(int status, int out_fd);
    ~HttpRequest();
    std::string get_data();
    std::string get_url();
private:
    int m_status;
    std::string m_data;
    std::string m_url;
    int m_request_type;
};

#endif