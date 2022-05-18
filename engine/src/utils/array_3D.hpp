#pragma once

#include <iostream>
#include <vector>

template<typename T>
class Array3D
{
    public:
        Array3D(uint32_t x, uint32_t y, uint32_t z): 
            width(x), height(y), data(x * y * z, 0) 
        {}
        
        int& operator()(uint32_t x, uint32_t y, uint32_t z)
        {
            return data.at(x + y * width + z * width * height);
        }
    private:
        uint32_t width, height;
        std::vector<T> data;
};