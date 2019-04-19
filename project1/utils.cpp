#include <string>
#include <vector>
#include <unordered_map>
#include <stdio.h>
#include "utils.h"
#include <iostream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

void convert_to_lowercase(std::string& upper){
    for(unsigned int i = 0; i<upper.length(); i++){
        upper[i] = tolower(upper[i]);
    }
}

std::unordered_map<std::string, std::string> get_filemap(){
    unordered_map<string, string> map;
    DIR* dirp = opendir(".");
    struct dirent * dp;
    struct stat statbuf;
    while ((dp = readdir(dirp))) {
        
        // check filetype. Only add if not a directory
        stat(dp->d_name, &statbuf);
        if(!S_ISREG(statbuf.st_mode))
            continue;

        string lower = dp->d_name;
        string filename = dp->d_name;
        convert_to_lowercase(lower);
        if(map.find(lower) == map.end()){
            map.insert(pair<string, string>(lower, filename));
        }
    }
    closedir(dirp);
    return map;
}

std::string strip(std::string src, std::string delimiter){
    if(delimiter.length() == 0)
        return src;
    while(src.find(delimiter) == 0){
        src = src.substr(delimiter.length(), src.size()- delimiter.length()+1);
    }

    while(src.rfind(delimiter) != string::npos && src.rfind(delimiter) == src.length() - delimiter.length()){
        src = src.substr(0, src.rfind(delimiter));
    }
    return src;
}

std::vector<std::string> split(std::string src, std::string delimiter){
    vector<string> v;
    src = strip(src, delimiter);
    while(1){
        src = strip(src, delimiter);
        int p1 = src.find(delimiter);
        if(p1 == -1){
            v.push_back(src);
            break;
        }
        v.push_back(src.substr(0, p1));
        src = src.substr(p1+1, src.length() -p1);
    }
    return v;
}

std::string convert_url_to_file(std::string url){
    convert_to_lowercase(url);
    if(url.compare("/") == 0){
        return "index.html";
    }
    if(url.length() == 1){
        return "";
    }
    string result = "";
    for(unsigned long i = 1; i<url.length(); i++){
        if(url[i] == '%' && i+2 < url.length() && url[i+1] == '2' && url[i+2] == '0'){
            result.push_back(' ');
            i += 2;
            continue;
        }
        result.push_back(url[i]);
    }
    return result;
}

std::string get_status_message(int code){
    switch(code){
        case 200:
            return "OK";
        case 400:
            return "Bad Request";
        case 404:
            return "Not Found";
        case 500:
            return "Internal Server Error";
        default:
            return "I'm a teapot";
    }
}

std::string get_content_type(std::string filename){
    int dotpos = filename.rfind(".");
    if(dotpos == -1){
        return "application/octet-stream";
    }
    string extension = filename.substr(dotpos, filename.length() - dotpos);
    convert_to_lowercase(extension);
    if(extension.compare(".html") == 0)
        return "text/html";
    if(extension.compare(".htm") == 0)
        return "text/html";
    if(extension.compare(".txt") == 0)
        return "text/plain";
    if(extension.compare(".jpg") == 0)
        return "image/jpeg";
    if(extension.compare(".jpeg") == 0)
        return "image/jpeg";
    if(extension.compare(".png") == 0)
        return "image/png";
    if(extension.compare(".gif") == 0)
        return "image/gif";
    if(extension.compare(".cpp") == 0)
        return "text/plain";
    return "application/octet-stream";
}