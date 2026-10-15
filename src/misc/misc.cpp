#include <string>
#include <iostream>
#include "tc/misc/misc.hpp"

void Tc::Log(std::string msg)
{
#ifdef TC_DEBUG_OUTPUT
    std::cout << msg << std::endl;
#endif
}

bool Tc::IsPointInBox(Vec2 point, Vec2 lb, Vec2 rt)
{
    return point.X > lb.X && point.X < rt.X && point.Y > lb.Y && point.Y < rt.Y;
}

float Tc::Clamp(float v, float min, float max)
{
    return std::min(std::max(v, min), max);
}

//converts the given angle to 0 - 2*PI range
float Tc::NormalizeAngle(float angle)
{
    if (angle < 0.0f)
        angle += 2.0f * PI;

    return fmodf(angle, 2.0f * PI);
}

//normalizes and subtracts 'b' from 'a'
float Tc::SubtractAngles(float a, float b)
{
    return NormalizeAngle(a) - NormalizeAngle(b);
}
