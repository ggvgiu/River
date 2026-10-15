#include "Renderer.h"
#include <gl/GL.h>
#include <cmath>

#pragma comment(lib, "opengl32.lib")

Renderer::~Renderer()
{
    Shutdown();
}

void Renderer::Shutdown()
{
    if (renderingContext_)
    {
        wglMakeCurrent(nullptr, nullptr);
        wglDeleteContext(renderingContext_);
        renderingContext_ = nullptr;
    }
    if (deviceContext_)
    {
        ReleaseDC(window_, deviceContext_);
        deviceContext_ = nullptr;
    }
    window_ = nullptr;
}

bool Renderer::Initialize(HWND window)
{
    Shutdown();
    window_ = window;
    deviceContext_ = GetDC(window_);
    if (!deviceContext_) return false;

    PIXELFORMATDESCRIPTOR format = {};
    format.nSize = sizeof(format);
    format.nVersion = 1;
    format.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    format.iPixelType = PFD_TYPE_RGBA;
    format.cColorBits = 32;
    format.cDepthBits = 24;
    format.iLayerType = PFD_MAIN_PLANE;
    const int pixelFormat = ChoosePixelFormat(deviceContext_, &format);
    if (!pixelFormat || !SetPixelFormat(deviceContext_, pixelFormat, &format))
    {
        Shutdown();
        return false;
    }
    renderingContext_ = wglCreateContext(deviceContext_);
    if (!renderingContext_ || !wglMakeCurrent(deviceContext_, renderingContext_))
    {
        Shutdown();
        return false;
    }
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
    glClearColor(0.025f, 0.035f, 0.065f, 1.0f);

    return true;
}

void Renderer::Resize(int width, int height)
{
    width_ = width;
    height_ = height;
}

void Renderer::Render(const RiverRaid::RiverRaidGame& game, const RenderOptions& options)
{
    if (!renderingContext_ || width_ <= 0 || height_ <= 0) return;
    glViewport(0, 0, width_, height_);
    glClearColor(0.015f, 0.025f, 0.04f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Preserve the Atari television's 4:3 picture instead of stretching gameplay.
    int viewWidth = width_;
    int viewHeight = width_ * 3 / 4;
    if (viewHeight > height_) { viewHeight = height_; viewWidth = height_ * 4 / 3; }
    glViewport((width_ - viewWidth) / 2, (height_ - viewHeight) / 2, viewWidth, viewHeight);
    view_.Draw(game, options);
    SwapBuffers(deviceContext_);
}
