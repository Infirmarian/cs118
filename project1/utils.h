#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <unordered_map>
#include <vector>
void convert_to_lowercase(std::string& upper);
void load_filemap(std::unordered_map<std::string, std::string>& map);
void split(std::string src, std::string delimiter, std::vector<std::string>& v);
std::string strip(std::string src, std::string delimiter);
std::string convert_url_to_file(std::string url);
std::string get_status_message(int code);
std::string get_content_type(std::string filename);
#endif