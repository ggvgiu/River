#pragma once
#include "Renderer.h"
#include "GameInput.h"
#include "GameAudio.h"
#include "RiverRaidGame.h"
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
    void Draw() { renderer_.Render(game_); }
    bool HandleMenu(HWND window, HINSTANCE instance, WORD command);
    void SetSuspended(bool value) { suspended_ = value; ResetClock(); }
private:
    Renderer renderer_;
    GameInput input_;
    RiverRaid::RiverRaidGame game_;
    GameAudio audio_;
    bool soundEnabled_ = true;
    bool suspended_ = false;
    double accumulator_ = 0;
    std::chrono::steady_clock::time_point lastUpdate_ = std::chrono::steady_clock::now();
};
