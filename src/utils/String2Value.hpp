#include <iostream>
#include <sstream>
#include <string>

template<class T>
inline T fromString(const std::string &str){
    std::istringstream is(str);
    T v;                                     
    is>>v;                          
    return v;                
}
