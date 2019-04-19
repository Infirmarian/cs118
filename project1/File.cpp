#include <string>
#include <fstream>
#include <ctime>
#include <sys/stat.h>
#include <cstring>
#include <iostream>

#include "File.h"
using namespace std;

File::File(std::string filename){
    m_file_exists = true;
    if (filename.compare("") == 0) {
        filename = "404.html";
        m_file_exists = false;
    }
    m_filestream.open(filename, ios::binary | ios::ate);
    m_filename = filename;
    if (m_filestream.fail()) {
        cerr << "Error: filestream failed.";
        exit(2);
    }

    m_file_size = m_filestream.tellg();
    m_filestream.seekg(0);

    stat(m_filename.c_str(), &m_stats);
}
File::~File(){
    m_filestream.close();
}
std::ifstream* File::get_file_stream(){
    return &m_filestream;
}
bool File::file_exists(){
    return m_file_exists;
}
int File::file_size(){
    return m_file_size;
}
std::string File::get_filename(){
    return m_filename;
}
struct tm* File::get_mod_time() {
    m_modtime = gmtime(&(m_stats.st_mtime));
    return m_modtime;
}
