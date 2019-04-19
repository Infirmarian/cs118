#include <vector>
#include <string>
#include "File.h"

#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H
class HttpResponse{
public:
    HttpResponse(int out_fd, File* file);
    ~HttpResponse();
    int flush_and_close();
private:
    int m_status;
    int m_ostream;
    std::ostringstream* m_stringstream;
    File* m_file;
    void format_header(std::ostringstream* ss);
};

#endif
