#pragma once

#define _USE_MATH_DEFINES // for C++
#include <cmath>

namespace engine
{
    namespace utils
    {
        inline float clamp(float value, float min, float max)
        {
            return std::fmax(min, std::fmin(max, value));
        }

        inline double degrees_to_radians(double degrees)
        {
            return degrees * M_PI / 180.0;
        }

        inline double radians_to_degrees(double radians)
        {
            return radians * 180.0 / M_PI;
        }
    }
}