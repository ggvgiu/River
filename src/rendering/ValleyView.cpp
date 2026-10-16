#include "platform/framework.h"
#include "rendering/ValleyView.h"
#include "rendering/visuals/AssetDrawing.h"
#include "rendering/TerrainSurface.h"
#include "rendering/visuals/EnvironmentPalette.h"
#include <gl/GL.h>
#include <algorithm>
#include <cmath>

namespace
{
    using namespace RiverRaid::Visuals;
    void Explosion(float x,float y,float altitude,int ticks,bool mono)
    {
        for(int i=0;i<14;++i)
        {
            const float angle=float(i)*6.2831853f/14;
            const float radius=3+float((90-ticks)%24)*0.3f;
            Box(x+std::cos(angle)*radius,y+std::sin(angle)*radius,1.5f,1.5f,
                altitude+float(i%3),altitude+float(i%3)+1.5f,{1,0.35f+float(i%3)*0.23f,0.1f},mono);
        }
    }
    float GroundHeight(const RiverRaid::RiverRaidGame& game,float x,float y)
    {
        return TerrainSurface::Height(game,x,game.Scroll()+y);
    }
}

void ValleyView::Draw(const RiverRaid::RiverRaidGame& game,const RenderOptions& options, const RiverRaid::Visuals::AssetLibrary& assets) const
{
    using namespace RiverRaid;
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    GLint viewport[4];glGetIntegerv(GL_VIEWPORT,viewport);
    // Reserve the existing bottom 20% for the unchanged, screen-space HUD.
    const int hud=viewport[3]/5;
    glViewport(viewport[0],viewport[1]+hud,viewport[2],viewport[3]-hud);
    glEnable(GL_SCISSOR_TEST);
    glScissor(viewport[0],viewport[1]+hud,viewport[2],viewport[3]-hud);
    const bool mono=game.Monochrome();
    Color sky=PaletteForSeed(game.BaseSeed(),options.seedColors).sky;
    if(mono) sky.r=sky.g=sky.b=0.299f*sky.r+0.587f*sky.g+0.114f*sky.b;
    glClearColor(sky.r,sky.g,sky.b,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LESS);
    glDisable(GL_BLEND);glDisable(GL_CULL_FACE);glShadeModel(GL_FLAT);
    glMatrixMode(GL_PROJECTION);glPushMatrix();glLoadIdentity();
    const double aspect=double(viewport[2])/std::max(1,viewport[3]-hud);
    constexpr double nearPlane=4;
    const double top=std::tan(55.0*3.141592653589793/360)*nearPlane;
    glFrustum(-top*aspect,top*aspect,-top,top,nearPlane,400);
    glMatrixMode(GL_MODELVIEW);glPushMatrix();glLoadIdentity();
    const float pitch=options.cameraAngle==CameraAngle::Low?25.0f:options.cameraAngle==CameraAngle::High?60.0f:40.0f;
    const float radians=pitch*3.14159265f/180;
    // Camera behind the plane, looking upstream (-Z). It follows lateral motion.
    // Rotating about X replaces GLU's look-at helper for this fixed heading.
    const float eyeY=7+90*std::tan(radians);
    glRotatef(pitch,1,0,0);
    glTranslatef(-(game.JetX()+4),-eyeY,-55);
    const GLfloat fog[]={sky.r,sky.g,sky.b,1};
    glEnable(GL_FOG);glFogi(GL_FOG_MODE,GL_LINEAR);glFogfv(GL_FOG_COLOR,fog);
    // End fog before the last available row: no new terrain generation or RNG calls.
    // Eye-space depth depends on pitch; scale fog to the ground's far edge.
    const float farDepth=std::sin(radians)*eyeY+std::cos(radians)*(55+155);
    const float nearDepth=std::sin(radians)*(eyeY-7)+std::cos(radians)*(55+12);
    glFogf(GL_FOG_START,nearDepth+(farDepth-nearDepth)*0.48f);
    glFogf(GL_FOG_END,farDepth-3);
    glHint(GL_FOG_HINT,GL_NICEST);

    TerrainSurface::Draw(game,mono,options.seedColors);
    for(const auto& section:game.Sections())
    for(const auto& object:section.course.Objects())
    {
        const float y=section.start+object.distance-game.Scroll();
        const auto& model=assets.Object(object.kind);
        const auto& frame=model.Frame(game.Frame());
        if(y+frame.length < -ValleyHistoryLines || y>=FieldHeight) continue;
        if(object.destroyed && !object.explosionTicks)continue;
        float altitude=model.altitude;
        if(object.kind==ObjectKind::Plane || object.kind==ObjectKind::Helicopter)
            altitude=TerrainSurface::FlightAltitude(game,frame,object.x,
                section.start+object.distance,model.altitude,
                model.mirrorsWithDirection && object.direction<0);
        if(object.destroyed)
        {
            if(model.groundMounted) altitude=GroundHeight(game,object.x+frame.width/2,y+frame.length/2)+3;
            if(object.explosionTicks) Explosion(object.x+frame.width/2.0f,y+frame.length/2.0f,altitude,object.explosionTicks,mono);
            continue;
        }
        const bool mirror=model.mirrorsWithDirection && object.direction<0;
        if(model.groundMounted)
        {
            const auto grounded=TerrainSurface::GroundFrame(game,frame,object.x,
                section.start+object.distance,mirror);
            DrawModel(grounded,object.x,y,altitude,mirror,mono);
        }
        else
        {
            DrawModelShadow(frame,object.x,y,0.05f,mirror,mono);
            DrawModel(frame,object.x,y,altitude,mirror,mono);
        }
    }
    if(game.Bullet().active) Box(game.Bullet().x,game.Bullet().y,1,3,7.5f,8.3f,{1,0.95f,0.5f},mono);
    if(game.State()==Phase::Crashing)
        Explosion(game.JetX()+4,12,std::max(7.0f,GroundHeight(game,game.JetX()+4,12)+3),game.CrashTicks(),mono);
    else if(game.State()!=Phase::GameOver)
    {
        const auto& model=assets.Player(game.Banking()!=0,game.ActivePlayer());
        const auto& frame=model.Frame(game.Frame());
        const float ground=GroundHeight(game,game.JetX()+4,12);
        // With invincibility, keep the cosmetic model visible above land it can
        // now cross. Altitude is a drawing detail, never a gameplay coordinate.
        const float altitude=game.Cheats().invincible?std::max(model.altitude,ground+3):model.altitude;
        DrawModelShadow(frame,game.JetX(),float(JetBottom),ground+0.05f,game.Banking()<0,mono);
        DrawModel(frame,game.JetX(),float(JetBottom),altitude,game.Banking()<0,mono);
    }
    glPopMatrix();glMatrixMode(GL_PROJECTION);glPopMatrix();glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
}
