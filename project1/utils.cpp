#include <string>
#include <vector>
#include <stdio.h>
#include "utils.h"

using namespace std;

void convert_to_lowercase(std::string& upper){
    for(unsigned int i = 0; i<upper.length(); i++){
        upper[i] = tolower(upper[i]);
    }
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