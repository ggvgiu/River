#include "platform/framework.h"
#include "rendering/RiverRaidView.h"
#include "game/CollisionShapes.h"
#include "game/GameRules.h"
#include "rendering/TerrainSurface.h"
#include "rendering/visuals/VisualNoise.h"
#include "rendering/visuals/EnvironmentPalette.h"
#include <gl/GL.h>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <vector>
#include <chrono>
#include <set>

#pragma comment(lib, "opengl32.lib")

namespace RiverRaid
{
    struct GameTestAccess
    {
        static void SetScroll(RiverRaidGame& game,float scroll) {game.scroll_=scroll;}
        static void ShowMountainCourse(RiverRaidGame& game)
        {
            game.Reset();game.phase_=Phase::Flying;
            game.sections_.clear();
            game.sections_.push_back({0,RiverCourse(2,game.BaseSeed()),2});
        }
        static void ShowHouse(RiverRaidGame& game,bool visible)
        {
            game.Reset();
            if(game.Monochrome()) game.ToggleColor();
            game.phase_=Phase::Flying;
            for(auto& section:game.sections_) section.course.Objects().clear();
            if(visible) game.sections_.front().course.Objects().push_back({ObjectKind::House,12,30,1});
        }
        static void ShowEnemyModels(RiverRaidGame& game)
        {
            game.Reset();
            if(game.Monochrome()) game.ToggleColor();
            game.scroll_=100;
            game.phase_=Phase::Flying;
            for(auto& section:game.sections_) section.course.Objects().clear();
            auto& objects=game.sections_.front().course.Objects();
            objects.push_back({ObjectKind::Ship,56,170,1});
            objects.push_back({ObjectKind::Helicopter,78,185,1});
            objects.push_back({ObjectKind::Plane,97,200,1});
            objects.push_back({ObjectKind::Fuel,70,140,1});
        }
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
int main(int argc,char** argv)
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
    // A right turn uses the unmirrored model; a left turn mirrors its geometry.
    // In each case the wing on the turning side must be lower.
    for(int player=0;player<2;++player)
    for(bool banking:{false,true})
    for(bool mirrored:{false,true})
    {
        const auto& frame=view.Assets().Player(banking,player).Frame(0);
        float left=0,right=0; int wings=0;
        for(const auto& box:frame.boxes)
            if(box.forward==10 && box.width==2.5f)
            {
                const float center=mirrored ? frame.width-box.x-box.width/2 : box.x+box.width/2;
                if(center<frame.width/2) left=box.top; else right=box.top;
                ++wings;
            }
        if(wings!=2 || (!banking && left!=right) ||
            (banking && (mirrored ? left>=right : right>=left)))
        {std::cerr<<"FAIL: player plane banking direction\n";return 11;}
    }
    RenderOptions options;
    if(argc>1 && std::string(argv[1])=="--benchmark")
    {
        RiverRaid::RiverRaidGame benchmarkGame;
        RiverRaid::GameTestAccess::ShowEnemyModels(benchmarkGame);
        const auto& flightFrame=view.Assets().Object(RiverRaid::ObjectKind::Helicopter).Frame(0);
        const auto flightStart=std::chrono::steady_clock::now();
        float flightSum=0;
        for(int i=0;i<300;++i)
            flightSum+=TerrainSurface::FlightAltitude(benchmarkGame,flightFrame,float(20+i%120),185,7,false);
        const auto flightElapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-flightStart).count();
        std::cout<<"Helicopter terrain sampling: "<<flightElapsed/300<<" ms/call (checksum "<<flightSum<<")\n";
        for(int i=0;i<3;++i) view.Draw(benchmarkGame,options);
        glFinish();
        const auto start=std::chrono::steady_clock::now();
        for(int i=0;i<30;++i) {view.Draw(benchmarkGame,options);glFinish();}
        const auto elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
        std::cout<<"Offscreen software OpenGL: "<<elapsed/30<<" ms/frame\n";
        benchmarkGame.ToggleInvincibility();
        std::array<PilotInput,2> benchmarkInput{};benchmarkInput[0].throttle=1;
        const auto movingStart=std::chrono::steady_clock::now();
        for(int i=0;i<60;++i) {benchmarkGame.Tick(benchmarkInput);view.Draw(benchmarkGame,options);glFinish();}
        const auto movingElapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-movingStart).count();
        std::cout<<"Scrolling software OpenGL: "<<movingElapsed/60<<" ms/frame\n";
        return 0;
    }
    if(!options.valley3D || options.cameraAngle!=CameraAngle::High) return 12;
    options.valley3D=false;
    RiverRaid::RiverRaidGame game;
    const auto initialCourseSeed=game.Sections().back().course.NextSeed();
    const auto baseSeed=game.BaseSeed();
    const float noise=RiverRaid::Visuals::Perlin(-1.23f,4.56f,baseSeed);
    if(noise!=RiverRaid::Visuals::Perlin(-1.23f,4.56f,baseSeed) ||
        noise==RiverRaid::Visuals::Perlin(-1.23f,4.56f,baseSeed^0x1234u)) return 19;
    const float landHeight=TerrainSurface::Height(game,-20,100);
    if(landHeight==TerrainSurface::Height(game,-20,140)) return 20;
    if(game.BaseSeed()!=baseSeed || game.Sections().back().course.NextSeed()!=initialCourseSeed) return 21;
    const auto& enemyPlane=view.Assets().Object(RiverRaid::ObjectKind::Plane).Frame(0);
    for(bool mirrored:{false,true})
    {
        const float overLand=TerrainSurface::FlightAltitude(game,enemyPlane,0,100,7,mirrored);
        const float overRiver=TerrainSurface::FlightAltitude(game,enemyPlane,76,100,7,mirrored);
        if(overLand<30 || overRiver!=7) return 17;
        float previous=overLand;
        for(float x=0.25f;x<=76;x+=0.25f)
        {
            const float flightHeight=TerrainSurface::FlightAltitude(game,enemyPlane,x,100,7,mirrored);
            if(std::abs(flightHeight-previous)>1) return 18;
            previous=flightHeight;
        }
    }
    // Scenery elevations depend on world coordinates, never the scrolling grid.
    const auto& house=view.Assets().Object(RiverRaid::ObjectKind::House).Frame(0);
    for(bool mirrored:{false,true})
    {
        const auto before=TerrainSurface::GroundFrame(game,house,12,100,mirrored);
        RiverRaid::GameTestAccess::SetScroll(game,1);
        const auto after=TerrainSurface::GroundFrame(game,house,12,100,mirrored);
        if(before.boxes.size()!=after.boxes.size() || before.boxes.size()<=house.boxes.size()) return 13;
        for(size_t i=0;i<before.boxes.size();++i)
            if(before.boxes[i].bottom!=after.boxes[i].bottom || before.boxes[i].top!=after.boxes[i].top) return 14;
        for(const auto& box:before.boxes)
            if(box.top<box.bottom) return 15;
        RiverRaid::GameTestAccess::SetScroll(game,0);
    }
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
    options.valley3D=true;
    RiverRaid::GameTestAccess::ShowHouse(game,false);
    bool passed=save("artifacts/valley-without-house.ppm");
    const auto withoutHouse=lastFrame;
    RiverRaid::GameTestAccess::ShowHouse(game,true);
    passed=save("artifacts/valley-house.ppm")&&passed;
    const auto renderedSeed=game.Sections().back().course.NextSeed();
    const auto stillFrame=lastFrame;
    passed=save("artifacts/valley-house-repeat.ppm")&&passed;
    if(lastFrame!=stillFrame || game.Sections().back().course.NextSeed()!=renderedSeed || game.BaseSeed()!=baseSeed) return 22;
    game.SetBaseSeed(baseSeed^1);
    RiverRaid::GameTestAccess::ShowHouse(game,true);
    passed=save("artifacts/valley-house-other-seed.ppm")&&passed;
    if(lastFrame==stillFrame) return 23;
    game.SetBaseSeed(baseSeed);
    RiverRaid::GameTestAccess::ShowHouse(game,true);
    passed=save("artifacts/valley-house-restored-seed.ppm")&&passed;
    if(lastFrame!=stillFrame) {std::cerr<<"FAIL: stale terrain cache after changing seed\n";return 24;}
    size_t housePixels=0;
    for(size_t i=0;i<lastFrame.size();i+=3)
        if(lastFrame[i]!=withoutHouse[i] || lastFrame[i+1]!=withoutHouse[i+1] || lastFrame[i+2]!=withoutHouse[i+2]) ++housePixels;
    if(housePixels<100) {std::cerr<<"FAIL: house hidden inside terrain\n";return 16;}
    game.Reset();
    options.valley3D=false;
    passed=save("artifacts/ready.ppm")&&passed;
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
    // Swap a visible model for custom artwork + mesh. Gameplay must never depend
    // on renderer-owned sizes, colors, geometry, or animation selection.
    game.Reset();
    const auto collisionSnapshot=[] {
        std::vector<int> result;
        auto append=[&](RiverRaid::CollisionShape shape) {
            result.push_back(shape.Width()); result.push_back(shape.Height());
            for(bool mirrored:{false,true})
                for(int y=0;y<shape.Height();++y)
                    for(int x=0;x<shape.Width();++x) result.push_back(shape.Solid(x,y,mirrored));
        };
        append(RiverRaid::JetCollisionShape(false)); append(RiverRaid::JetCollisionShape(true));
        for(auto kind:{RiverRaid::ObjectKind::Ship,RiverRaid::ObjectKind::Helicopter,
            RiverRaid::ObjectKind::Plane,RiverRaid::ObjectKind::Fuel,RiverRaid::ObjectKind::House,RiverRaid::ObjectKind::Bridge})
        {
            for(int tick:{0,8}) append(RiverRaid::ObjectCollisionShape(kind,tick));
            result.push_back(RiverRaid::ObjectScore(kind));
        }
        return result;
    };
    const auto originalCollisions=collisionSnapshot();
    passed=save("artifacts/model-original-2d.ppm")&&passed;
    const auto original2D=lastFrame;
    options.valley3D=true;
    passed=save("artifacts/model-original-3d.ppm")&&passed;
    const auto original3D=lastFrame;
    const auto originalModel=view.Assets().Player(false,0);
    RiverRaid::Visuals::AssetModel replacement;
    replacement.altitude=9;
    RiverRaid::Visuals::AssetFrame replacementFrame;
    replacementFrame.width=12; replacementFrame.length=18;
    replacementFrame.pixels.push_back({0,8,12,{1,0,1}});
    replacementFrame.boxes.push_back({0,7,12,6,0,3,{1,0,1}});
    replacementFrame.triangles.push_back({{{{0,3,10},{12,3,10},{6,6,18}}},{0,1,1}});
    replacement.frames.push_back(replacementFrame);
    view.Assets().SetPlayer(false,0,replacement);
    passed=save("artifacts/model-replacement-3d.ppm")&&passed;
    if(lastFrame==original3D) return 8;
    options.valley3D=false;
    passed=save("artifacts/model-replacement-2d.ppm")&&passed;
    if(lastFrame==original2D || originalCollisions!=collisionSnapshot()) return 9;
    auto reference=game;
    inputs={};inputs[0].fire=true;
    for(int i=0;i<120;++i) {game.Tick(inputs);reference.Tick(inputs);}
    if(game.Scroll()!=reference.Scroll() || game.JetX()!=reference.JetX() || game.Fuel()!=reference.Fuel() ||
        game.State()!=reference.State() || game.Players()[0].score!=reference.Players()[0].score ||
        game.Players()[0].lives!=reference.Players()[0].lives ||
        game.Sections().back().course.NextSeed()!=reference.Sections().back().course.NextSeed()) return 10;
    view.Assets().SetPlayer(false,0,originalModel);
    RiverRaid::GameTestAccess::ShowEnemyModels(game);
    options.valley3D=true;
    options.cameraAngle=CameraAngle::Medium;
    passed=save("artifacts/enemies-and-carrier.ppm")&&passed;
    options.cameraAngle=CameraAngle::Low;
    passed=save("artifacts/enemies-and-carrier-low.ppm")&&passed;
    game.Reset();
    RiverRaid::GameTestAccess::ShowMountainCourse(game);
    bool foundIsland=false;
    for(int world=0;world<900;++world)
    {
        const auto row=game.Row(float(world));
        if(row.island<10)continue;
        const auto seedBefore=game.Sections().back().course.NextSeed();
        const float center=TerrainSurface::Height(game,80,float(world));
        if(center==12 || TerrainSurface::Height(game,80-row.island,float(world))!=12 ||
            TerrainSurface::Height(game,80+row.island,float(world))!=12) return 25;
        RiverRaid::GameTestAccess::SetScroll(game,float(world-50));
        options.cameraAngle=CameraAngle::High;
        passed=save("artifacts/valley-island-relief.ppm")&&passed;
        if(TerrainSurface::Height(game,80,float(world))!=center ||
            game.Sections().back().course.NextSeed()!=seedBefore) return 26;
        foundIsland=true;break;
    }
    if(!foundIsland) {std::cerr<<"FAIL: island test scene missing\n";return 27;}
    using namespace RiverRaid::Visuals;
    if(&PaletteForSeed(RiverRaid::DefaultSeed,true)!=&EnvironmentPalettes[0]) return 28;
    std::array<bool,EnvironmentPalettes.size()> covered{};covered[0]=true;
    std::set<const EnvironmentPalette*> uniquePalettes;
    for(unsigned seed=1;seed<=65535;++seed)uniquePalettes.insert(&PaletteForSeed(uint16_t(seed),true));
    if(uniquePalettes.size()!=1+(EnvironmentPalettes.size()-1)*PaletteVariants)return 33;
    std::set<std::array<float,12>> uniqueColors;
    for(const auto* p:uniquePalettes)
        uniqueColors.insert({p->water.r,p->water.g,p->water.b,p->grass.r,p->grass.g,p->grass.b,
            p->rock.r,p->rock.g,p->rock.b,p->sky.r,p->sky.g,p->sky.b});
    if(uniqueColors.size()!=uniquePalettes.size())return 34;
    for(uint16_t seed=1;seed<1000;++seed)
    {
        const size_t index=PaletteThemeIndex(seed,true);
        if(&PaletteForSeed(seed,false)!=&EnvironmentPalettes[0] || index==0) return 29;
        if(covered[index])continue;
        covered[index]=true;
        game.SetBaseSeed(seed);
        RiverRaid::GameTestAccess::ShowEnemyModels(game);
        const auto unchangedSeed=game.Sections().back().course.NextSeed();
        for(bool valley:{false,true})
        {
            options.valley3D=valley;options.seedColors=true;
            const std::string filename="artifacts/palette-"+std::to_string(index)+(valley?"-3d.ppm":"-2d.ppm");
            passed=save(filename.c_str())&&passed;
            const auto colored=lastFrame;
            options.seedColors=false;
            passed=save("artifacts/palette-disabled.ppm")&&passed;
            if(lastFrame==colored) return 30;
            options.seedColors=true;
            passed=save(filename.c_str())&&passed;
            if(lastFrame!=colored || game.BaseSeed()!=seed || game.Sections().back().course.NextSeed()!=unchangedSeed) return 31;
        }
    }
    for(bool theme:covered)if(!theme)return 32;
    wglMakeCurrent(nullptr,nullptr);wglDeleteContext(context);
    SelectObject(dc,old);DeleteObject(dib);DeleteDC(dc);
    if(passed) std::cout<<"PASS: rendering, trailing history, swappable models, unchanged collision masks and gameplay\n";
    return passed?0:3;
}
