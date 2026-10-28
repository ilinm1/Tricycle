#pragma once

#include <iostream>
#include <string>
#include "vec2.hpp"

//various methods that I don't know where to put

namespace Tc
{
    inline void Log(std::string msg)
    {
#ifdef TC_DEBUG_OUTPUT
        std::cout << msg << std::endl;
#endif
    }

    inline bool IsPointInBox(Vec2 point, Vec2 lb, Vec2 rt)
    {
        return point.X > lb.X && point.X < rt.X && point.Y > lb.Y && point.Y < rt.Y;
    }

    inline float Clamp(float v, float min, float max)
    {
        return std::min(std::max(v, min), max);
    }

    //converts the given angle to 0 - 2*PI range
    inline float NormalizeAngle(float angle)
    {
        if (angle < 0.0f)
            angle += 2.0f * PI;

        return fmodf(angle, 2.0f * PI);
    }

    //normalizes and subtracts 'b' from 'a'
    inline float SubtractAngles(float a, float b)
    {
        return NormalizeAngle(a) - NormalizeAngle(b);
    }

}
