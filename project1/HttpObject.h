#include <vector>
#include <string>

#ifndef HTTPOBJECT_H
#define HTTPOBJECT_H
class HttpObject{
public:
    HttpObject(int stream_fd);
    HttpObject(int status, int out_fd);
    ~HttpObject();
    std::string get_data();
    std::string get_url();
private:
    int m_status;
    std::string m_data;
    std::string m_url;
    int m_request_type;
};

#endif