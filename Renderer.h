#pragma once
#include "framework.h"
#include "RiverRaidView.h"


// Owns the OpenGL context and renders the world into a Windows drawing surface.
class Renderer
{
public:
    Renderer() = default;
    ~Renderer();
    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool Initialize(HWND window);
    void Shutdown();
    void Resize(int width, int height);
    void Render(const RiverRaid::RiverRaidGame& game, const RenderOptions& options);

private:
    HWND window_ = nullptr;
    HDC deviceContext_ = nullptr;
    HGLRC renderingContext_ = nullptr;
    int width_ = 640;
    int height_ = 480;

    RiverRaidView view_;
};
