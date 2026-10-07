//testing drawing, layer transparency, texture loading, sound loading & playback, mouse input

#include <thread>
#include <format>
#include <random>
#include "tc/audio.hpp"
#include "tc/graphics.hpp"
#include "tc/misc.hpp"

namespace Tcg = Tc::Graphics;
namespace Tca = Tc::Audio;

struct TriangleLayer : Tcg::Layer
{
    Tcg::Texture Texture;

    TriangleLayer() : Texture(Tcg::ResolveTexture("test.png"))
    {
        Redraw = true;
    }

    void Draw() override
    {
        if (!Redraw)
            return;

        DrawTriangle(Tc::Vec2(-0.75f, -0.75f), Tc::Vec2(0.75f, -0.75f), Tc::Vec2(0.0f, 0.75f), COLOR_TRANSPARENT, Texture);
    }
};

struct BallLayer : Tcg::Layer
{
    Tcg::Texture Texture;
    Tca::AudioFile HitSound;

    Tc::Color BallColor;
    Tc::Vec2 BallPos = Tc::Vec2(0.0f);
    Tc::Vec2 PrevBallPos = Tc::Vec2(0.0f);
    Tc::Vec2 BallVelocity;

    float TotalTime = 0.0f;
    bool Dragging = false;
    bool PrevDragging = false;

    Tc::Color GradientColor1 = Tc::Color(255, 0, 0, 128);
    Tc::Color GradientColor2 = Tc::Color(0, 0, 255, 128);
    const float GradientTime = 10.0f;

    const Tc::Vec2 BallSize = Tc::Vec2(0.5f);
    const float InitialBallSpeed = 0.5f;
    const float HitSpeedMultiplier = 0.95f;
    const float TimeStep = 0.1f;
    
    BallLayer() : Tcg::Layer(true, GL_TRIANGLES, TCG_HEIGHT_MAX), Texture(Tcg::ResolveTexture("test.png")), HitSound(Tca::LoadFile("test.ogg"))
    {
        std::default_random_engine engine;
        engine.seed(std::time(nullptr));
        std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
        distribution.reset();
        BallVelocity = Tc::Vec2::FromAngle(2.0f * PI * distribution(engine)) * InitialBallSpeed;

        Subscribe<Tcg::MousePressEvent>(&OnMousePress);
    }

    Tc::Color GetGradientColor(float t)
    {
        t = fmod(t / GradientTime, 1.0f);

        if (t <= TimeStep / GradientTime)
        {
            Tc::Color oldColor = GradientColor1;
            GradientColor1 = GradientColor2;
            GradientColor2 = oldColor;
        }

        return GradientColor1 * (1.0f - t) + GradientColor2 * t;
    }

    static bool OnMousePress(Tcg::MousePressEvent& ev, void* data)
    {
        BallLayer* layerPtr = reinterpret_cast<BallLayer*>(data);
        Tc::Vec2 mousePos = Tcg::PointFromPixels(Tcg::GetCursorPos(), layerPtr->IsWorldSpace);
        if (ev.Action == GLFW_PRESS && Tc::IsPointInBox(mousePos, layerPtr->BallPos, layerPtr->BallPos + layerPtr->BallSize))
        {
            layerPtr->Dragging = true;
        }
        else
        {
            layerPtr->Dragging = false;
        }

        return true;
    }

    void Draw() override
    {
        if (Dragging)
        {
            PrevBallPos = BallPos;
            BallPos = Tcg::PointFromPixels(Tcg::GetCursorPos(), IsWorldSpace);
        }
        else
        {
            BallPos += BallVelocity * TimeStep;
        }

        if (PrevDragging && !Dragging)
            BallVelocity = (BallPos - PrevBallPos) / TimeStep / 2.0f; //dividing by two just to reduce it a bit
        PrevDragging = Dragging;

        bool hit = false;
        Tc::Vec2 bounds = Tcg::CameraSize / 2.0f;
        if (BallPos.X + BallSize.X > bounds.X || BallPos.X < -bounds.X)
        {
            BallPos.X = Tc::Clamp(BallPos.X, -bounds.X, bounds.X - BallSize.X);
            BallVelocity.X *= -1.0f;
            if (BallVelocity.Length() > InitialBallSpeed)
                BallVelocity *= HitSpeedMultiplier;
            hit = true;
        }

        if (BallPos.Y + BallSize.Y > bounds.Y || BallPos.Y < -bounds.Y)
        {
            BallPos.Y = Tc::Clamp(BallPos.Y, -bounds.Y, bounds.Y - BallSize.Y);
            BallVelocity.Y *= -1.0f;
            if (BallVelocity.Length() > InitialBallSpeed)
                BallVelocity *= HitSpeedMultiplier;
            hit = true;
        }

        if (hit)
        {
            if (HitSound.Playing)
            {
                Tca::SeekFile(&HitSound, 0);
            }
            else
            {
                Tca::PlayFile(&HitSound);
            }
        }

        TotalTime += TimeStep;
        BallColor = GetGradientColor(TotalTime);
        DrawRect(BallPos, BallPos + BallSize, BallColor, Texture);
    }
};

int main()
{
    Tca::Initialize();
    Tcg::Initialize(300, 300, "Ball");
    Tcg::SetCameraSize(Tc::Vec2(3.0f));

    TriangleLayer triangleLayer = {};
    Tcg::AddLayer(&triangleLayer);

    BallLayer ballLayer = {};
    Tcg::AddLayer(&ballLayer);

    std::thread audioThread(Tca::UpdateLoop);
    Tcg::UpdateLoop();
    Tcg::Shutdown();
    Tca::Shutdown();
    audioThread.~thread();

    return 0;
}
