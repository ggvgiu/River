#include "framework.h"
#include "ValleyView.h"
#include "SpriteAssets.h"
#include <gl/GL.h>
#include <algorithm>
#include <cmath>

namespace
{
    struct Color { float r,g,b; };
    void Paint(Color c, bool monochrome, float shade=1)
    {
        if (monochrome) c.r=c.g=c.b=0.299f*c.r+0.587f*c.g+0.114f*c.b;
        glColor3f(c.r*shade,c.g*shade,c.b*shade);
    }
    void Box(float x,float forward,float width,float length,float bottom,float top,Color color,bool mono)
    {
        const float right=x+width, front=-forward, back=-(forward+length);
        glBegin(GL_QUADS);
        Paint(color,mono); // Top surface, with deliberately hard voxel edges.
        glVertex3f(x,top,front);glVertex3f(right,top,front);glVertex3f(right,top,back);glVertex3f(x,top,back);
        Paint(color,mono,0.57f);
        glVertex3f(x,bottom,front);glVertex3f(right,bottom,front);glVertex3f(right,top,front);glVertex3f(x,top,front);
        Paint(color,mono,0.78f);
        glVertex3f(x,bottom,back);glVertex3f(x,bottom,front);glVertex3f(x,top,front);glVertex3f(x,top,back);
        Paint(color,mono,0.66f);
        glVertex3f(right,bottom,front);glVertex3f(right,bottom,back);glVertex3f(right,top,back);glVertex3f(right,top,front);
        Paint(color,mono,0.72f);
        glVertex3f(right,bottom,back);glVertex3f(x,bottom,back);glVertex3f(x,top,back);glVertex3f(right,top,back);
        glEnd();
    }
    Color ObjectColor(RiverRaid::ObjectKind kind,int row)
    {
        using RiverRaid::ObjectKind;
        switch(kind)
        {
        case ObjectKind::Fuel: return (row/6)%2 ? Color{0.92f,0.25f,0.25f} : Color{0.91f,0.93f,0.83f};
        case ObjectKind::Ship: return row<10 ? Color{0.46f,0.73f,0.72f} : Color{0.93f,0.53f,0.25f};
        case ObjectKind::Helicopter: return row>13 ? Color{1,0.65f,0.22f} : Color{0.28f,0.86f,0.83f};
        case ObjectKind::Plane: return {0.64f,0.8f,0.95f};
        case ObjectKind::House: return row<13 ? Color{0.3f,0.67f,0.26f} : Color{0.96f,0.88f,0.71f};
        case ObjectKind::Bridge: return {0.72f,0.49f,0.24f};
        }
        return {1,1,1};
    }
    void VoxelSprite(RiverRaid::Sprite sprite,float x,float y,float altitude,float thickness,
        bool mirrored,Color color,bool mono,const RiverRaid::ObjectKind* kind=nullptr)
    {
        for(int row=0;row<sprite.Height();++row)
        {
            if(y+row < -ValleyHistoryLines || y+row>=RiverRaid::FieldHeight) continue;
            for(int column=0;column<sprite.Width();)
            {
                if(!sprite.Solid(column,row,mirrored)){++column;continue;}
                const int start=column++;
                while(column<sprite.Width() && sprite.Solid(column,row,mirrored)) ++column;
                Box(x+start,y+row,float(column-start),1,altitude,altitude+thickness,
                    kind?ObjectColor(*kind,row):color,mono);
            }
        }
    }
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
        const auto row=game.Row(game.Scroll()+y);
        if(row.island>0 && x>=80-row.island && x<=80+row.island) return 12;
        if(x>=row.bank && x<=160-row.bank) return 0;
        const float edge=std::floor(std::min(x,160-x)/8)*8;
        return 10+std::min(24.0f,std::max(0.0f,row.bank-edge)*0.4f);
    }
}

void ValleyView::Draw(const RiverRaid::RiverRaidGame& game,const RenderOptions& options) const
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
    Color sky{0.32f,0.43f,0.48f};
    if(mono) sky.r=sky.g=sky.b=0.299f*sky.r+0.587f*sky.g+0.114f*sky.b;
    glClearColor(sky.r,sky.g,sky.b,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);glDisable(GL_CULL_FACE);glShadeModel(GL_FLAT);
    glMatrixMode(GL_PROJECTION);glPushMatrix();glLoadIdentity();
    const double aspect=double(viewport[2])/std::max(1,viewport[3]-hud);
    constexpr double nearPlane=1;
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

    // Reuse completed course sections already retained by the game, including
    // their exact bank/island shapes and destroyed-object states. Negative
    // screen coordinates are behind the plane, not newly generated scenery.
    constexpr int apron = ValleyHistoryLines;
    Box(-80,-apron,320,FieldHeight+apron,-1,0,{0.06f,0.22f,0.38f},mono);
    for(int y=-apron;y<FieldHeight;y+=2)
    {
        const auto row=game.Row(game.Scroll()+y);
        // Stair-step terraces turn the original banks and islands into a valley.
        for(float x=0;x<row.bank;x+=8)
        {
            const float width=std::min(8.0f,row.bank-x);
            const float height=10+std::min(24.0f,(row.bank-x)*0.4f);
            Color land{0.24f,0.43f,0.16f};
            if(height>25) land={0.37f,0.4f,0.31f};
            if(row.road) land={0.4f,0.4f,0.36f};
            Box(x,float(y),width,2,0,height,land,mono);
            Box(160-x-width,float(y),width,2,0,height,land,mono);
        }
        if(row.island>0)
            Box(80-row.island,float(y),2*row.island,2,0,12,{0.28f,0.46f,0.17f},mono);
        // Shore highlights and occasional water dashes keep the retro style.
        Box(row.bank,float(y),0.5f,2,0,0.12f,{0.5f,0.62f,0.4f},mono);
        Box(159.5f-row.bank,float(y),0.5f,2,0,0.12f,{0.5f,0.62f,0.4f},mono);
        if((int(game.Scroll()+y)&15)==0)
            for(int x=24;x<144;x+=16)
                if(game.IsWater(float(x),game.Scroll()+y)) Box(float(x),float(y),2,0.5f,0.02f,0.04f,{0.14f,0.32f,0.46f},mono);
    }
    // Continue side walls laterally so no scene border is exposed near the camera.
    Box(-80,-apron,80,FieldHeight+apron,0,38,{0.3f,0.4f,0.22f},mono);
    Box(160,-apron,80,FieldHeight+apron,0,38,{0.3f,0.4f,0.22f},mono);
    for(const auto& section:game.Sections())
    for(const auto& object:section.course.Objects())
    {
        const float y=section.start+object.distance-game.Scroll();
        const auto sprite=ObjectSprite(object.kind,game.Frame());
        if(y+sprite.Height() < -ValleyHistoryLines || y>=FieldHeight) continue;
        float altitude=0.5f,thickness=2;
        if(object.kind==ObjectKind::Helicopter || object.kind==ObjectKind::Plane) altitude=7;
        if(object.kind==ObjectKind::House)
        {
            const auto row=game.Row(section.start+object.distance+14);
            const float edge=std::min(object.x,160-object.x-sprite.Width());
            altitude=row.island>0 && object.x>80-row.island && object.x<80+row.island ? 12 :
                10+std::min(24.0f,std::max(0.0f,row.bank-std::floor(edge/8)*8)*0.4f);
            thickness=3;
        }
        if(object.kind==ObjectKind::Fuel) thickness=3;
        if(object.kind==ObjectKind::Bridge) {altitude=5;thickness=3;}
        if(object.destroyed)
        {
            if(object.explosionTicks) Explosion(object.x+sprite.Width()/2.0f,y+sprite.Height()/2.0f,altitude,object.explosionTicks,mono);
            continue;
        }
        const bool mirror=object.kind!=ObjectKind::Fuel && object.direction<0;
        VoxelSprite(sprite,object.x,y,0.05f,0.02f,mirror,{0.035f,0.10f,0.15f},mono);
        VoxelSprite(sprite,object.x,y,altitude,thickness,mirror,{1,1,1},mono,&object.kind);
        if(object.kind==ObjectKind::Bridge)
        {
            Box(object.x,y,2,13,0,5,{0.48f,0.4f,0.29f},mono);
            Box(object.x+30,y,2,13,0,5,{0.48f,0.4f,0.29f},mono);
        }
    }
    if(game.Bullet().active) Box(game.Bullet().x,game.Bullet().y,1,3,7.5f,8.3f,{1,0.95f,0.5f},mono);
    if(game.State()==Phase::Crashing)
        Explosion(game.JetX()+4,12,std::max(7.0f,GroundHeight(game,game.JetX()+4,12)+3),game.CrashTicks(),mono);
    else if(game.State()!=Phase::GameOver)
    {
        const auto sprite=JetSprite(game.Banking()!=0);
        const float ground=GroundHeight(game,game.JetX()+4,12);
        // With invincibility, keep the cosmetic model visible above land it can
        // now cross. Altitude is a drawing detail, never a gameplay coordinate.
        const float altitude=game.Cheats().invincible?std::max(7.0f,ground+3):7;
        VoxelSprite(sprite,game.JetX(),float(JetBottom),ground+0.05f,0.02f,game.Banking()<0,{0.035f,0.10f,0.15f},mono);
        VoxelSprite(sprite,game.JetX(),float(JetBottom),altitude,1.4f,game.Banking()<0,
            game.ActivePlayer()==0?Color{0.98f,0.87f,0.23f}:Color{0.9f,0.92f,0.98f},mono);
    }
    glPopMatrix();glMatrixMode(GL_PROJECTION);glPopMatrix();glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
}
