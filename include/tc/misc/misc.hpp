#pragma once

#include <string>
#include "vec2.hpp"

//various methods that I don't know where to put

namespace Tc
{
	void Log(std::string msg);

	bool IsPointInBox(Vec2 point, Vec2 lb, Vec2 rt);

	float Clamp(float v, float min, float max);

	//converts the given angle to 0 - 2*PI range
	float NormalizeAngle(float angle);

	//normalizes and subtracts 'b' from 'a'
	float SubtractAngles(float a, float b);
}
