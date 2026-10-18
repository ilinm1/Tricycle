#include "tc/misc.hpp"
#include "test.hpp"

void Vec2Test()
{
    TCT_ASSERT(Tc::Vec2() == Tc::Vec2());
    TCT_ASSERT(Tc::Vec2() == Tc::Vec2(0.0f, 0.0f));
    TCT_ASSERT(Tc::Vec2(1.0f) == Tc::Vec2(1.0f, 1.0f));
    TCT_ASSERT(Tc::Vec2(2.0f, 3.0f) == Tc::Vec2(2.0f, 3.0f));
    TCT_ASSERT(Tc::Vec2(2.0f, 3.0f) != Tc::Vec2(3.0f, 2.0f));
    TCT_ASSERT(Tc::Vec2() + Tc::Vec2(1.0f) == Tc::Vec2(1.0f));
    TCT_ASSERT(Tc::Vec2(1.0f) - Tc::Vec2(1.0f) == Tc::Vec2());
    TCT_ASSERT(Tc::Vec2(2.0f, 3.0f) + Tc::Vec2(1.0f) == Tc::Vec2(3.0f, 4.0f));
    TCT_ASSERT(Tc::Vec2(2.0f, 3.0f) * 0.5f == Tc::Vec2(1.0f, 1.5f));
    TCT_ASSERT(Tc::Vec2(2.0f, 3.0f) * 0.5f == Tc::Vec2(2.0f, 3.0f) / 2.0f);

    TCT_ASSERT(Tc::Vec2().Abs() == Tc::Vec2());
    TCT_ASSERT(Tc::Vec2(-2.0f, 3.0f).Abs() == Tc::Vec2(2.0f, 3.0f));
    TCT_ASSERT(Tc::Vec2(2.0f, -3.0f).Abs() == Tc::Vec2(2.0f, 3.0f));

    TCT_ASSERT(Tc::Vec2::Max(Tc::Vec2(), Tc::Vec2()) == Tc::Vec2());
    TCT_ASSERT(Tc::Vec2::Max(Tc::Vec2(1.0f, 5.0f), Tc::Vec2(3.0f, 3.0f)) == Tc::Vec2(3.0f, 5.0f));
    TCT_ASSERT(Tc::Vec2::Max(Tc::Vec2(3.0f, 3.0f), Tc::Vec2(1.0f, 5.0f)) == Tc::Vec2(3.0f, 5.0f));

    TCT_ASSERT(Tc::Vec2::Min(Tc::Vec2(), Tc::Vec2()) == Tc::Vec2());
    TCT_ASSERT(Tc::Vec2::Min(Tc::Vec2(1.0f, 5.0f), Tc::Vec2(3.0f, 3.0f)) == Tc::Vec2(1.0f, 3.0f));
    TCT_ASSERT(Tc::Vec2::Min(Tc::Vec2(3.0f, 3.0f), Tc::Vec2(1.0f, 5.0f)) == Tc::Vec2(1.0f, 3.0f));

    TCT_ASSERT(Tc::Vec2().Round() == Tc::Vec2());
    TCT_ASSERT(Tc::Vec2(0.1f, 0.5f).Round() == Tc::Vec2(0.0f, 1.0f));
    TCT_ASSERT(Tc::Vec2(0.5f, 0.1f).Round() == Tc::Vec2(1.0f, 0.0f));

    TCT_ASSERT(Tc::Vec2().Floor() == Tc::Vec2());
    TCT_ASSERT(Tc::Vec2(0.5f, 1.5f).Floor() == Tc::Vec2(0.0f, 1.0f));
    TCT_ASSERT(Tc::Vec2(1.5f, 0.5f).Floor() == Tc::Vec2(1.0f, 0.0f));

    TCT_ASSERT(Tc::Vec2().Ceil() == Tc::Vec2());
    TCT_ASSERT(Tc::Vec2(0.5f, 1.5f).Ceil() == Tc::Vec2(1.0f, 2.0f));
    TCT_ASSERT(Tc::Vec2(1.5f, 0.5f).Ceil() == Tc::Vec2(2.0f, 1.0f));

    const float tolerance = 1E-3;
    for (float length = 0.01f; length < 10.0f; length += 0.01f)
    {
        for (float angle = 0.0f; angle < 2.0f * PI; angle += 0.01f)
        {
            Tc::Vec2 v = Tc::Vec2(cosf(angle), sinf(angle)) * length;
            TCT_ASSERT(std::fabsf(v.Length() - length) < tolerance);
            TCT_ASSERT(std::fabsf(Tc::SubtractAngles(v.ToAngle(), angle)) < tolerance);
            TCT_ASSERT((v.Normalized() - Tc::Vec2::FromAngle(angle)).Length() < tolerance);
            TCT_ASSERT(std::fabsf(Tc::SubtractAngles(v.Rotated(PI / 2.0f).ToAngle(), angle + PI / 2.0f)) < tolerance);
            TCT_ASSERT(std::fabsf(v.Dotp(v) - length * length) < tolerance);
            TCT_ASSERT(std::fabsf(v.Dotp(v.Rotated(PI / 2.0f)) < tolerance));
        }
    }
}
TCT_REGISTER(Vec2Test);

void Mat3Test()
{
    TCT_ASSERT(Tc::Mat3() == Tc::Mat3());
    TCT_ASSERT(SINGLE_ARG(Tc::Mat3 { 3.0f, 2.0f, 1.0f, 2.0f, 1.0f, 0.0f, -1.0f, 5.0f, 0.5f } == Tc::Mat3 { 3.0f, 2.0f, 1.0f, 2.0f, 1.0f, 0.0f, -1.0f, 5.0f, 0.5f }));
    TCT_ASSERT(SINGLE_ARG(Tc::Mat3 { 3.0f, 2.0f, 1.0f, 2.0f, 1.0f, 0.0f, -1.0f, 5.0f, 0.5f } != Tc::Mat3 { 3.0f, 2.0f, 1.0f, 2.0f, 1.0f, 0.0f, 1.0f, 5.0f, 0.5f }));

    TCT_ASSERT(MAT3_IDENTITY.TransformVector(Tc::Vec2(2.0f, 3.0f)) == Tc::Vec2(2.0f, 3.0f));
    TCT_ASSERT(MAT3_IDENTITY.TransformVector(Tc::Vec2(-1.0f, 2.0f)) == Tc::Vec2(-1.0f, 2.0f));

    const float tolerance = 1E-5;
    Tc::Mat3 mat { 2.0f, 3.0f, 0.0f, -1.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f };
    TCT_ASSERT(SINGLE_ARG((mat.TransformVector(Tc::Vec2(3.0f, 2.0f)) - Tc::Vec2(12.0f, -1.0f)).Length() < tolerance));
    TCT_ASSERT(SINGLE_ARG((mat.TransformVector(Tc::Vec2(2.0f, 3.0f)) - Tc::Vec2(13.0f, 0.5f)).Length() < tolerance));

    TCT_ASSERT(SINGLE_ARG(Tc::Mat3 { 3.0f, 2.0f, 1.0f, 2.0f, 1.0f, 0.0f, -1.0f, 5.0f, 0.5f }.AsColumnMajor() == Tc::Mat3 { 3.0f, 2.0f, -1.0f, 2.0f, 1.0f, 5.0f, 1.0f, 0.0f, 0.5f }));
}
TCT_REGISTER(Mat3Test);
