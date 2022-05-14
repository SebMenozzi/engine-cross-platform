#pragma once

// https://en.wikipedia.org/wiki/C_data_types

// 8 bytes
typedef unsigned char uint8; // 0 => 255
typedef signed char sint8; // −127 => 127

// 16 bytes
typedef unsigned short uint16; // 0 => 65 535
typedef signed short sint16; // −32 767 => 32 767

// 32 bytes
typedef unsigned long uint32; // 0 => 4 294 967 295
typedef signed long sint32; // −2 147 483 647 => 2 147 483 64

// Complex type
#include <complex>
typedef std::complex<double> complex;

struct Buffer
{
    const uint8* data_ = nullptr;
    uint32 data_stride_ = 0;
    uint32 nb_elements_ = 0;
};