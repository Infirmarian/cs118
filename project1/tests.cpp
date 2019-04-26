//tests.cpp
// This file runs tests that were used in dev to check specific functions
#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <unordered_map>

#include "utils.h"
using namespace std;

bool vector_equal(vector<string> one, vector<string> two){
    if(one.size() != two.size())
    return false;
    for(unsigned int i = 0; i< one.size(); i++){
        if(one[i].compare(two[i]))
            return false;
    }
    return true;
}
void print_vector(vector<string> v){
    for(unsigned int i = 0; i < v.size()-1; i++){
        cout<<v[i]<<",";
    }
    cout<<v[v.size()-1]<<endl;
}
void check_strip_sub(string s, string d, string e){
    strip(s,d);
    assert(s.compare(e)==0);
}

void check_strip(){
    check_strip_sub("","","");
    check_strip_sub("", " ", "");
    check_strip_sub("a", "a", "");
    check_strip_sub("aba", "a", "b");
    check_strip_sub("whwhlalawh", "wh", "lala");
    check_strip_sub("xyyz", "xy", "yz");
    check_strip_sub("xyyz", "y", "xyyz");
    check_strip_sub("xyyz", "", "xyyz");
    check_strip_sub("  what a great day  ", " ", "what a great day");
}

void check_split(){
    vector<string>* v = split("this is a space delimited string", " ");
    vector<string> g;
    g.push_back("this");
    g.push_back("is");
    g.push_back("a");
    g.push_back("space");
    g.push_back("delimited");
    g.push_back("string");
    assert(vector_equal(*v, g));
    v = split("this is a space delimited string ", " ");
    assert(vector_equal(*v, g));
    v = split("    this    is    a  space delimited   string  ", " ");
    assert(vector_equal(*v, g));
}

void check_conversion(){
    assert(convert_url_to_file("/").compare("index.html") == 0);
    assert(convert_url_to_file("/somepath").compare("somepath") == 0);
    assert(convert_url_to_file("/some/long/path").compare("some/long/path") == 0);
    assert(convert_url_to_file("/gasp%20a%20space!").compare("gasp a space!") == 0);
    assert(convert_url_to_file("/index.html").compare("index.html") == 0);
}

void check_status_strings(){
    assert(get_status_message(200).compare("OK") == 0);
    assert(get_status_message(404).compare("Not Found") == 0);
    assert(get_status_message(500).compare("Internal Server Error") == 0);
    assert(get_status_message(600).compare("I'm a teapot") == 0);
}

void check_filetype_id(){
    assert(get_content_type("file.html").compare("text/html") == 0);
    assert(get_content_type("index.htm").compare("text/html") == 0);
    assert(get_content_type("plain.txt").compare("text/plain") == 0);
    assert(get_content_type("meme.jpg").compare("image/jpeg") == 0);
    assert(get_content_type("long.file.name.jpeg").compare("image/jpeg") == 0);
    assert(get_content_type("embedded/high/quality.gif").compare("image/gif") == 0);
    assert(get_content_type("WhaTDoYOuMemE.TxT").compare("text/plain") == 0);

}

void check_filemap_acquisition(){
    unordered_map<string, string> s;
    load_filemap(s);
    assert(s.size() > 0);
    assert(s.find("tests.cpp") != s.end());
    assert(s.find("utils.cpp") != s.end());
}

int main(){
    check_strip();
    check_split();
    check_conversion();
    check_status_strings();
    check_filetype_id();
    check_filemap_acquisition();
    cout<<"All tests passed"<<endl;
    return 0;
}