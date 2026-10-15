#pragma once

#include <cmath>

#define PI 3.141592

namespace Tc
{
    struct Vec2
    {
        float X;
        float Y;

        Vec2() : X(0), Y(0) {};

        Vec2(float xy) : X(xy), Y(xy) {}

        Vec2(float x, float y) : X(x), Y(y) {}

        //returns vector with maximal component values of both input vectors
        static Vec2 Max(Vec2 a, Vec2 b)
        {
            return Vec2(std::max(a.X, b.X), std::max(a.Y, b.Y));
        }

        //returns vector with minimal component values of both input vectors
        static Vec2 Min(Vec2 a, Vec2 b)
        {
            return Vec2(std::min(a.X, b.X), std::min(a.Y, b.Y));
        }

        //in radians
        static Vec2 FromAngle(float angle)
        {
            return Vec2(cosf(angle), sinf(angle));
        }

        //returns vector's angle in radians; however it is NOT in 0 - 2*PI range, you can use 'NormalizeAngle' to convert it if needed
        float ToAngle()
        {
            if (X == 0.0f)
                return Y > 0.0f ? PI / 2.0f : PI / -2.0f;

            float v = atanf(Y / X);
            return X > 0.0f ? v : v + PI;
        }

        Vec2 Round()
        {
            return Vec2(std::roundf(X), std::roundf(Y));
        }

        Vec2 Floor()
        {
            return Vec2(std::floorf(X), std::floorf(Y));
        }

        Vec2 Ceil()
        {
            return Vec2(std::ceilf(X), std::ceilf(Y));
        }

        Vec2 Rotated(float angle)
        {
            return Vec2(X * cosf(angle) - Y * sinf(angle), Y * cosf(angle) + X * sinf(angle));
        }

        float Length()
        {
            return sqrtf(X * X + Y * Y);
        }

        float Dotp(const Vec2& v)
        {
            return X * v.X + Y * v.Y;
        }

        Vec2 Normalized()
        {
            float l = Length();
            return Vec2(X / l, Y / l);
        }

        Vec2 Abs()
        {
            return Vec2{ abs(X), abs(Y) };
        }

        Vec2 operator+(const Vec2& other)
        {
            return Vec2(X + other.X, Y + other.Y);
        }

        Vec2& operator+=(const Vec2& other)
        {
            *this = *this + other;
            return *this;
        }

        Vec2 operator-(const Vec2& other)
        {
            return Vec2(X - other.X, Y - other.Y);
        }

        Vec2& operator-=(const Vec2& other)
        {
            *this = *this - other;
            return *this;
        }

        Vec2 operator*(const float& other)
        {
            return Vec2(X * other, Y * other);
        }

        Vec2& operator*=(const float& other)
        {
            *this = *this * other;
            return *this;
        }

        Vec2 operator/(const float& other)
        {
            return Vec2(X / other, Y / other);
        }

        Vec2& operator/=(const float& other)
        {
            *this = *this / other;
            return *this;
        }

        bool operator==(const Vec2& other)
        {
            return X == other.X && Y == other.Y;
        }

        bool operator!=(const Vec2& other)
        {
            return !(*this == other);
        }
    };
}
