#pragma once
#include <sstream>
#include <iostream>


#ifdef WARN_LOG_ENABLED
inline void warn_log(const char * str){
    std::cout << "[WARNING]: "<< str << std::endl;
}
#else 
inline void warn_log(const char * str){
}
#endif

#ifdef INFO_LOG_ENABLED
inline void info_log(const char * str){
    std::cout << "[INFO]: " << str;
}
#else
inline void info_log(const char* str){

}
#endif