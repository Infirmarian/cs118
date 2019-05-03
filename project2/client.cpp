#include <iostream>


int main(int argc, char** argv){
    if(argc != 4){
        std::cerr<<"Bad argument count, got "<<argc<<" expected 4";
        exit(1);
    }
    std::cout<<"Hello client!"<<std::endl;
}