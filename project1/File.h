#include <string>
#include <fstream>
#include <sys/stat.h>
#include <unistd.h>
#include <ctime>
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
    struct tm* get_mod_time();
private:
    std::ifstream m_filestream;
    bool m_file_exists;
    int m_file_size;
    std::string m_filename;
    struct stat m_stats;
    struct tm* m_modtime;
};

#endif
