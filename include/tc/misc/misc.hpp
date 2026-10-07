#pragma once

#include "vec2.hpp"

//various methods that I don't know where to put

namespace Tc
{
	void Log(std::string msg);
	bool IsPointInBox(Vec2 point, Vec2 lb, Vec2 rt);
	float Clamp(float v, float min, float max);
}
