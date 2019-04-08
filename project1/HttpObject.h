#ifndef HTTPOBJECT_H
#define HTTPOBJECT_H
class HttpObject{
public:
    HttpObject(int stream_fd);
    int status;
private:
    int m_status;
    int m_data;
};

#endif