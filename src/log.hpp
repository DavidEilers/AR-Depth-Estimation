#pragma once
#include <sstream>
#include <iostream>

#define ERROR_LOG_ENABLED
#define WARN_LOG_ENABLED
//#define INFO_LOG_ENABLED

namespace arDepthEstimation{

class Logger{
    const std::string m_prefix;
    std::ostream*  m_stream;
    const bool m_enabled;


    public: 
    Logger(std::string prefix_, std::ostream* stream_, bool enabled_) : m_prefix{prefix_}, m_stream{stream_}, m_enabled{enabled_}
    {

    }


    template <typename T>
    Logger operator <<( T out){
        if(m_enabled){
            (*m_stream) << m_prefix << out << std::endl;
            return *this;
        }
        return *this;
    }

};

#ifdef ERROR_LOG_ENABLED
Logger logger_error{"\033[31m[ERROR]\033[0m", (&std::cout), true};
#else
Logger logger_error{"\033[31m[ERROR]\033[0m", (&std::cout), false};
#endif

#ifdef WARN_LOG_ENABLED
Logger logger_warn{"\033[33m[Warning]\033[0m", (&std::cout), true};
#else
Logger logger_warn{"\033[33m[Warning]\033[0m",(&std::cout), false};
#endif

#ifdef INFO_LOG_ENABLED
Logger logger_info{"[Info]",(&std::cout), true};
#else
Logger logger_info{"[Info]",(&std::cout), false};
#endif

}
