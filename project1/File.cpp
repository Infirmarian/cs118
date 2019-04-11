#include <string>
#include <fstream>

#include "File.h"
using namespace std;

File::File(std::string filename){
    m_filestream.open(filename, ios::binary | ios::ate);
    m_filename = filename;
    if (m_filestream.fail()) {
        m_file_exists = false;
        m_file_size = 0;
    }else{
        m_file_exists = true;
        m_file_size = m_filestream.tellg();
        m_filestream.seekg(0);
    }
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