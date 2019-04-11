#include <string>
#include <fstream>
#ifndef FILE_H
#define FILE_H
class File{
public:
    File(std::string filename);
    ~File();
    bool file_exists();
    int file_size();
    std::string get_filename();
    std::ifstream* get_file_stream();
private:
    std::ifstream m_filestream;
    bool m_file_exists;
    int m_file_size;
    std::string m_filename;
};

#endif