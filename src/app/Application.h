#pragma once
#include "rendering/Renderer.h"
#include "input/GameInput.h"
#include "audio/GameAudio.h"
#include "game/RiverRaidGame.h"
#include <chrono>

// Coordinates input, fixed-step gameplay, audio, and presentation.
class Application
{
public:
    bool Initialize(HWND window);
    void Shutdown();
    void Update(bool focused);
    void ResetClock();
    void Resize(int width, int height) { renderer_.Resize(width, height); }
    void Draw() { renderer_.Render(game_, renderOptions_); }
    bool HandleMenu(HWND window, HINSTANCE instance, WORD command);
    void SetSuspended(bool value) { suspended_ = value; ResetClock(); }
private:
    Renderer renderer_;
    RenderOptions renderOptions_;
    GameInput input_;
    RiverRaid::RiverRaidGame game_;
    GameAudio audio_;
    bool soundEnabled_ = true;
    bool suspended_ = false;
    double accumulator_ = 0;
    std::chrono::steady_clock::time_point lastUpdate_ = std::chrono::steady_clock::now();
};
