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
    // Check if the file is a valid file
    // If not, filename will equal ""
    // Return path to 404.html if not a valid file
    if (filename.compare("") == 0) {
        filename = "404.html";
        m_file_exists = false;
    }
    // Attempt to open the file
    m_filestream.open(filename, ios::binary | ios::ate);
    m_filename = filename;
    // Check to make sure the file opened correctly
    if (m_filestream.fail()) {
        cerr << "Error: filestream failed.";
        exit(2);
    }

    // Set the file size
    m_file_size = m_filestream.tellg();
    m_filestream.seekg(0);

    // Store the stats of the file to be used for last modified date
    stat(m_filename.c_str(), &m_stats);
}
File::~File(){
    // Close file when done
    m_filestream.close();
}
std::ifstream* File::get_file_stream(){
    // Return the file stream to the caller
    return &m_filestream;
}
bool File::file_exists(){
    // Return if the file exists to the caller
    return m_file_exists;
}
int File::file_size(){
    // Return the file size to the caller
    return m_file_size;
}
std::string File::get_filename(){
    // Return the file name to the caller
    return m_filename;
}
struct tm* File::get_mod_time() {
    // Get the last modified time using the stats saved previously
    m_modtime = gmtime(&(m_stats.st_mtime));
    // Return the last modified time to the caller for the HTTP Response
    return m_modtime;
}
