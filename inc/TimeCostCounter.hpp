/**
 * @file TimeCostCounter.hpp
 * @author ZXW2600 (zhaoxinwei74@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-09-29
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include <chrono>
#include <iostream>

class TimeCostCounter
{
private:
    std::chrono::time_point<std::chrono::system_clock> start;
    std::chrono::time_point<std::chrono::system_clock> end;
    std::chrono::duration<double> elapsed;

public:
    TimeCostCounter()
    {
        start = std::chrono::system_clock::now();
    }
    void elapse()
    {
        end = std::chrono::system_clock::now();
        elapsed = end - start;
        std::cout << "Elapsed time: " << elapsed.count() *1000 << "ms" << std::endl;
    }
    void begin()
    {
        start = std::chrono::system_clock::now();
    }
    ~TimeCostCounter()
    {
        elapse();
    }
};