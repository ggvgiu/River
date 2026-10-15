#include "../framework.h"
#include "../RiverRaidView.h"
#include <gl/GL.h>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <vector>

#pragma comment(lib, "opengl32.lib")

namespace RiverRaid
{
    struct GameTestAccess
    {
        static void AddPassedHelicopter(RiverRaidGame& game)
        {
            auto& section=game.sections_.front();
            // All occupied pixels are below the old 2D bottom boundary. The
            // high camera can still see this location behind the aircraft.
            section.course.Objects().push_back({ObjectKind::Helicopter,76,
                game.scroll_-section.start-26,1});
        }
    };
}

// Draw the same view into a Win32 DIB, so visual QA works without capturing
// the user's desktop (including remote/headless execution sessions).
int main()
{
    std::filesystem::create_directories("artifacts");
    constexpr int width=960,height=720;
    HDC dc=CreateCompatibleDC(nullptr);
    BITMAPINFO bitmap{};
    bitmap.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    bitmap.bmiHeader.biWidth=width;
    bitmap.bmiHeader.biHeight=height;
    bitmap.bmiHeader.biPlanes=1;
    bitmap.bmiHeader.biBitCount=32;
    bitmap.bmiHeader.biCompression=BI_RGB;
    void* pixels=nullptr;
    HBITMAP dib=CreateDIBSection(dc,&bitmap,DIB_RGB_COLORS,&pixels,nullptr,0);
    HGDIOBJ old=SelectObject(dc,dib);
    PIXELFORMATDESCRIPTOR format{};
    format.nSize=sizeof(format);format.nVersion=1;
    format.dwFlags=PFD_DRAW_TO_BITMAP|PFD_SUPPORT_OPENGL|PFD_SUPPORT_GDI;
    format.iPixelType=PFD_TYPE_RGBA;format.cColorBits=32;format.cDepthBits=24;
    int chosen=ChoosePixelFormat(dc,&format);
    if(!chosen || !SetPixelFormat(dc,chosen,&format)) return 1;
    HGLRC context=wglCreateContext(dc);
    if(!context || !wglMakeCurrent(dc,context)) return 2;
    glViewport(0,0,width,height);
    glClearColor(0,0,0,1);
    RiverRaidView view;
    RenderOptions options;
    RiverRaid::RiverRaidGame game;
    std::vector<unsigned char> lastFrame;
    auto save=[&](const char* filename) {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        view.Draw(game, options);
        glFinish();
        GLenum error=glGetError();
        if(error!=GL_NO_ERROR) {std::cerr<<"OpenGL error: "<<error<<'\n';return false;}
        std::vector<unsigned char> rgb(width*height*3);
        glPixelStorei(GL_PACK_ALIGNMENT,1);
        glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,rgb.data());
        lastFrame=rgb;
        size_t nonBlack=0;
        for(auto value:rgb) if(value) ++nonBlack;
        if(nonBlack<rgb.size()/10) {std::cerr<<"Empty framebuffer\n";return false;}
        std::ofstream out(filename,std::ios::binary);
        out<<"P6\n"<<width<<' '<<height<<"\n255\n";
        for(int y=height-1;y>=0;--y) out.write(reinterpret_cast<const char*>(rgb.data()+y*width*3),width*3);
        return bool(out);
    };
    bool passed=save("artifacts/ready.ppm");
    std::array<PilotInput,2> inputs{};
    inputs[0].fire=true;
    for(int i=0;i<160;++i) game.Tick(inputs);
    passed=save("artifacts/playing.ppm")&&passed;
    const auto originalFrame=game.Frame();
    const auto originalScroll=game.Scroll();
    const auto originalFuel=game.Fuel();
    options.valley3D=true;
    options.cameraAngle=CameraAngle::Low;
    passed=save("artifacts/valley-low.ppm")&&passed;
    options.cameraAngle=CameraAngle::Medium;
    passed=save("artifacts/valley-medium.ppm")&&passed;
    options.cameraAngle=CameraAngle::High;
    passed=save("artifacts/valley-high.ppm")&&passed;
    options.valley3D=false;
    passed=save("artifacts/back-to-2d.ppm")&&passed;
    if(game.Frame()!=originalFrame || game.Scroll()!=originalScroll || game.Fuel()!=originalFuel) return 4;
    options.valley3D=true;
    options.cameraAngle=CameraAngle::Medium;
    game.ToggleInvincibility();
    inputs[0].throttle=1;
    for(int i=0;i<650;++i) game.Tick(inputs);
    passed=save("artifacts/valley-mountains.ppm")&&passed;
    game.ToggleColor();
    passed=save("artifacts/valley-monochrome.ppm")&&passed;
    game.ToggleColor();
    game.TogglePause();
    passed=save("artifacts/valley-paused.ppm")&&passed;
    game.TogglePause();
    game.Reset();
    inputs={}; inputs[0].throttle=1;
    // Invincibility remains selected across reset: pass an intact bridge and
    // render it at negative screen Y, across a river/mountains boundary.
    while(game.Scroll()<1040) game.Tick(inputs);
    const auto historyFrame=game.Frame();
    const auto historyScroll=game.Scroll();
    const auto historySeed=game.Sections().back().course.NextSeed();
    const auto oldRow=game.Row(1000);
    bool pastBridge=false;
    for(const auto& section:game.Sections())
        for(const auto& object:section.course.Objects())
            if(object.kind==RiverRaid::ObjectKind::Bridge && !object.destroyed &&
                section.start+object.distance-game.Scroll() < -32) pastBridge=true;
    if(!pastBridge) return 5;
    options.cameraAngle=CameraAngle::High;
    passed=save("artifacts/valley-history.ppm")&&passed;
    const auto beforePassedObject=lastFrame;
    RiverRaid::GameTestAccess::AddPassedHelicopter(game);
    passed=save("artifacts/passed-helicopter.ppm")&&passed;
    if(lastFrame==beforePassedObject)
    {
        std::cerr<<"FAIL: object behind the 2D boundary did not change the framebuffer\n";
        return 7;
    }
    options.valley3D=false;
    passed=save("artifacts/history-2d.ppm")&&passed;
    options.valley3D=true;
    passed=save("artifacts/history-back-to-3d.ppm")&&passed;
    if(game.Frame()!=historyFrame || game.Scroll()!=historyScroll ||
        game.Sections().back().course.NextSeed()!=historySeed || game.Row(1000).bank!=oldRow.bank) return 6;
    options.valley3D=false;
    game.SelectPlayers();game.TogglePause();
    passed=save("artifacts/paused.ppm")&&passed;
    game.ToggleColor();
    passed=save("artifacts/monochrome.ppm")&&passed;
    wglMakeCurrent(nullptr,nullptr);wglDeleteContext(context);
    SelectObject(dc,old);DeleteObject(dib);DeleteDC(dc);
    if(passed) std::cout<<"PASS: OpenGL 2D, 3D camera angles, return to 2D, overlays; gameplay state unchanged\n";
    return passed?0:3;
}
