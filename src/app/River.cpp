#include "platform/framework.h"
#include "app/River.h"
#include "app/Application.h"
#include "app/MenuActions.h"

namespace
{
    constexpr UINT_PTR AnimationTimer = 1;
    HINSTANCE instance;
}

LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    auto* app = reinterpret_cast<Application*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE)
    {
        const auto* creation = reinterpret_cast<const CREATESTRUCTW*>(lParam);
        app = static_cast<Application*>(creation->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    }
    switch (message)
    {
    case WM_CREATE:
        if (!app->Initialize(window) || !SetTimer(window, AnimationTimer, 16, nullptr))
        {
            KillTimer(window, AnimationTimer);
            app->Shutdown();
            MessageBoxW(window, L"Could not initialize the OpenGL drawing surface.", L"River", MB_OK | MB_ICONERROR);
            return -1;
        }
        return 0;
    case WM_SIZE:
        app->Resize(LOWORD(lParam), HIWORD(lParam));
        InvalidateRect(window, nullptr, FALSE);
        return 0;
    case WM_TIMER:
        if (wParam == AnimationTimer)
        {
            app->Update(GetForegroundWindow() == window && !IsIconic(window));
            if (!IsIconic(window)) InvalidateRect(window, nullptr, FALSE);
        }
        return 0;
    case WM_ACTIVATEAPP:
        if (app) app->ResetClock();
        return 0;
    case WM_ENTERMENULOOP:
    case WM_ENTERSIZEMOVE:
        app->SetSuspended(true);
        return 0;
    case WM_EXITMENULOOP:
    case WM_EXITSIZEMOVE:
        app->SetSuspended(false);
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT:
        {
            PAINTSTRUCT paint;
            BeginPaint(window, &paint);
            app->Draw();
            EndPaint(window, &paint);
        }
        return 0;
    case WM_COMMAND:
        if (app->HandleMenu(window, instance, LOWORD(wParam))) return 0;
        break;
    case WM_DESTROY:
        KillTimer(window, AnimationTimer);
        app->Shutdown();
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(window, message, wParam, lParam);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE,
                     _In_ LPWSTR, _In_ int nCmdShow)
{
    instance = hInstance;
    // Keep the initial drawing area at 640 x 480 physical pixels on scaled displays.
    SetProcessDPIAware();
    WCHAR title[100] = {};
    WCHAR className[100] = {};
    LoadStringW(instance, IDS_APP_TITLE, title, 100);
    LoadStringW(instance, IDC_RIVER, className, 100);

    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = WndProc;
    windowClass.hInstance = instance;
    windowClass.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(IDI_RIVER));
    windowClass.hIconSm = LoadIconW(instance, MAKEINTRESOURCEW(IDI_SMALL));
    windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    windowClass.lpszMenuName = MAKEINTRESOURCEW(IDC_RIVER);
    windowClass.lpszClassName = className;
    if (!RegisterClassExW(&windowClass)) return 1;

    constexpr DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
    RECT bounds = { 0, 0, 640, 480 };
    if (!AdjustWindowRectEx(&bounds, style, TRUE, 0)) return 1;
    Application app;
    HWND window = CreateWindowExW(0, className, title, style, CW_USEDEFAULT, CW_USEDEFAULT,
        bounds.right - bounds.left, bounds.bottom - bounds.top, nullptr, nullptr, instance, &app);
    if (!window) return 1;
    ShowWindow(window, nCmdShow);
    UpdateWindow(window);

    HACCEL accelerators = LoadAcceleratorsW(instance, MAKEINTRESOURCEW(IDC_RIVER));
    MSG message = {};
    int result;
    while ((result = static_cast<int>(GetMessageW(&message, nullptr, 0, 0))) > 0)
    {
        if (!TranslateAcceleratorW(window, accelerators, &message))
        {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    return result == -1 ? 1 : static_cast<int>(message.wParam);
}
