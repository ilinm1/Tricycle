#pragma once

#include "vec2.hpp"

namespace Tc
{
    //3x3 matrix (last three cells are unused)
    struct Mat3
    {
        float Cells[9] = { 0 };

        Vec2 TransformVector(Vec2 v)
        {
            return Vec2(v.Dotp(*reinterpret_cast<Vec2*>(Cells)) + Cells[2], v.Dotp(*reinterpret_cast<Vec2*>(Cells + 3)) + Cells[5]);
        }

        //elements in glsl matrices are stored contiguosly in columns (as opposed to rows)
        Mat3 AsColumnMajor()
        {
            return Mat3{ Cells[0], Cells[3], Cells[6], Cells[1], Cells[4], Cells[7], Cells[2], Cells[5], Cells[8] };
        }

        bool operator==(const Mat3&) const = default;
    };

    #define MAT3_IDENTITY Tc::Mat3 { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f }
}
