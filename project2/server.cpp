#include <iostream>


int main(int argc, char** argv){
    if(argc != 2){
        std::cerr<<"Bad argument count, got "<<argc<<" expected 2"<<std::endl;
        exit(1);
    }
    int port = atoi(argv[1]);
    std::cout<<"Hello server!"<<std::endl;
}