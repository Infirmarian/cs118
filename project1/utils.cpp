#include <string>
#include "utils.h"

using namespace std;

void convert_to_lowercase(std::string& upper){
    for(unsigned int i = 0; i<upper.length(); i++){
        upper[i] = tolower(upper[i]);
    }
}
