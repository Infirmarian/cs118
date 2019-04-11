#include <vector>
#include <string>
#include "File.h"

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H
class HttpResponse{
public:
    HttpResponse(int out_fd, File* file);
    ~HttpResponse();
    int flush_and_close(bool close_connection);
private:
    int m_status;
    int m_ostream;
    File* m_file;
    std::string format_header(bool close_connection);
};

#endif