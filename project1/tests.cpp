//tests.cpp
#include <iostream>
#include <vector>
#include <string>
#include <cassert>


#include "utils.h"
using namespace std;
bool vector_equal(vector<string> one, vector<string> two){
    if(one.size() != two.size())
    return false;
    for(int i = 0; i< one.size(); i++){
        if(one[i].compare(two[i]))
            return false;
    }
    return true;
}
void print_vector(vector<string> v){
    for(int i = 0; i < v.size()-1; i++){
        cout<<v[i]<<",";
    }
    cout<<v[v.size()-1]<<endl;
}

void check_strip(){
    assert(strip("", "").compare("") == 0);
    assert(strip("", " ").compare("") == 0);
    assert(strip("a", "a").compare("") == 0);
    assert(strip("aba", "a").compare("b") == 0);
    assert(strip("whwhlalawh", "wh").compare("lala") == 0);
    assert(strip("xyyz", "xy").compare("yz") == 0);
    assert(strip("xyyz", "y").compare("xyyz") == 0);
    assert(strip("xyyz", "").compare("xyyz") == 0);
    assert(strip("  what a great day  ", " ").compare("what a great day") == 0);
}

void check_split(){
    vector<string> v = split("this is a space delimited string", " ");
    vector<string> g;
    g.push_back("this");
    g.push_back("is");
    g.push_back("a");
    g.push_back("space");
    g.push_back("delimited");
    g.push_back("string");
    assert(vector_equal(v, g));
    v = split("this is a space delimited string ", " ");
    assert(vector_equal(v, g));
    v = split("    this    is    a  space delimited   string  ", " ");
    assert(vector_equal(v, g));
}

void check_conversion(){
    assert(convert_url_to_file("/").compare("") == 0);
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

int main(){
    check_strip();
    check_split();
    check_conversion();
    check_status_strings();
    cout<<"All tests passed"<<endl;
    return 0;
}