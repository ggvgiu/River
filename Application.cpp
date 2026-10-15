#include "Application.h"
#include "MenuActions.h"
#include <algorithm>

bool Application::Initialize(HWND window)
{
    if (!renderer_.Initialize(window)) return false;
    audio_.Initialize();
    MenuActions::UpdateChecks(window, game_, soundEnabled_);
    ResetClock();
    return true;
}

void Application::Shutdown()
{
    audio_.Shutdown();
    renderer_.Shutdown();
}

void Application::ResetClock()
{
    lastUpdate_ = std::chrono::steady_clock::now();
    accumulator_ = 0;
}

void Application::Update(bool focused)
{
    const auto now = std::chrono::steady_clock::now();
    const double elapsed = std::chrono::duration<double>(now - lastUpdate_).count();
    lastUpdate_ = now;
    const bool running = focused && !suspended_;
    if (running && !game_.Paused())
    {
        // The Win32 timer schedules work; it does not determine game speed.
        constexpr double frame = 1.0 / 60.0;
        accumulator_ += std::min(elapsed, 0.1);
        const auto controls = input_.Read(true);
        while (accumulator_ >= frame)
        {
            game_.Tick(controls);
            accumulator_ -= frame;
        }
    }
    else accumulator_ = 0;
    audio_.Pump(game_.Sound(), running && !game_.Paused() && soundEnabled_);
}

bool Application::HandleMenu(HWND window, HINSTANCE instance, WORD command)
{
    bool handled = MenuActions::HandleCommand(window, instance, command, game_, soundEnabled_);
    if (handled) ResetClock();
    return handled;
}
