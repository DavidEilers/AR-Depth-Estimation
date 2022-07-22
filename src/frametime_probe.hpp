#pragma once
#include "glfw.hpp"

#include <vector>
#include <limits>
#include <iomanip>

class FrameTimeProbe{
    double m_start_time;
    double m_stop_time;
    double m_min_time;
    double m_max_time;
    double m_avg_time;
    bool m_run_already;
    const std::string m_name;

    public: 
    FrameTimeProbe(const std::string name):
    m_min_time{std::numeric_limits<double>::max()},
    m_max_time{0},
    m_run_already{false},
    m_name{name}
    {
        
    }

    ~FrameTimeProbe(){};

    void start(){
        m_start_time = glfwGetTime();
    }

    void reset(){
        m_min_time = std::numeric_limits<double>::max();
        m_max_time = 0;
        m_run_already = false;
    }

    void stop(){
        m_stop_time = glfwGetTime();
        double timespan = m_stop_time - m_start_time;
        if(m_run_already){
            m_avg_time = (m_avg_time/2.0) + ((timespan)/2.0);
        }else{
            m_run_already = true;
            m_avg_time = timespan;
        }

        if(timespan > m_max_time) m_max_time = timespan;
        if(timespan < m_min_time) m_min_time = timespan;
    }

    std::string to_string(){
        std::stringstream  out{}; out << "[" << m_name << "]:" << std::endl << "min: " << std::setprecision(4) << m_min_time*1000 << "ms avg: " << m_avg_time*1000 << "ms max: " << m_max_time*1000 << "ms";
        return out.str();
    }

    std::string to_csv_line(std::string name){
        std::stringstream  out{}; out << name << "," << std::setprecision(4) << m_min_time*1000 << "," << m_avg_time*1000 << "," << m_max_time*1000;
        return out.str();
    }


};