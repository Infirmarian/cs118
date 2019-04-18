#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <unordered_map>
#include <vector>
void convert_to_lowercase(std::string& upper);
std::unordered_map<std::string, std::string> get_filemap();
std::vector<std::string> split(std::string src, std::string delimiter);
std::string strip(std::string src, std::string delimiter);
std::string convert_url_to_file(std::string url);
std::string get_status_message(int code);
std::string get_content_type(std::string filename);
#endif