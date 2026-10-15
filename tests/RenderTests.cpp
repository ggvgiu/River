#include "../framework.h"
#include "../RiverRaidView.h"
#include <gl/GL.h>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <vector>

#pragma comment(lib, "opengl32.lib")

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
    RiverRaid::RiverRaidGame game;
    auto save=[&](const char* filename) {
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        view.Draw(game);
        glFinish();
        GLenum error=glGetError();
        if(error!=GL_NO_ERROR) {std::cerr<<"OpenGL error: "<<error<<'\n';return false;}
        std::vector<unsigned char> rgb(width*height*3);
        glPixelStorei(GL_PACK_ALIGNMENT,1);
        glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,rgb.data());
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
    game.SelectPlayers();game.TogglePause();
    passed=save("artifacts/paused.ppm")&&passed;
    game.ToggleColor();
    passed=save("artifacts/monochrome.ppm")&&passed;
    wglMakeCurrent(nullptr,nullptr);wglDeleteContext(context);
    SelectObject(dc,old);DeleteObject(dib);DeleteDC(dc);
    if(passed) std::cout<<"PASS: OpenGL ready, gameplay, pause, monochrome framebuffers\n";
    return passed?0:3;
}
