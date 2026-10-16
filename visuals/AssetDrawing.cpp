#include "../framework.h"
#include "AssetDrawing.h"
#include "../RenderOptions.h"
#include "../RiverCourse.h"
#include <gl/GL.h>

namespace RiverRaid::Visuals
{
    void Paint(Color c, bool monochrome, float shade)
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
    namespace
    {
        void Rect(float x,float y,float width)
        {
            glBegin(GL_QUADS);
            glVertex2f(x,y); glVertex2f(x+width,y);
            glVertex2f(x+width,y+1); glVertex2f(x,y+1);
            glEnd();
        }
        float MirrorX(float x,float width,float frameWidth,bool mirrored)
        {
            return mirrored ? frameWidth-x-width : x;
        }
    }

    void DrawPixels(const AssetFrame& frame,float x,float y,bool mirrored,bool monochrome)
    {
        for(const auto& run:frame.pixels)
        {
            Paint(run.color,monochrome);
            Rect(x+MirrorX(run.x,run.width,frame.width,mirrored),y+run.y,run.width);
        }
    }

    void DrawPixelShadow(const AssetFrame& frame,float x,float y,bool mirrored,bool monochrome,Color color)
    {
        Paint(color,monochrome);
        for(const auto& run:frame.pixels)
            Rect(x+MirrorX(run.x,run.width,frame.width,mirrored),y+run.y,run.width);
    }

    void DrawModel(const AssetFrame& frame,float x,float y,float altitude,bool mirrored,bool monochrome)
    {
        for(const auto& part:frame.boxes)
        {
            if(y+part.forward+part.length < -ValleyHistoryLines || y+part.forward>=FieldHeight) continue;
            Box(x+MirrorX(part.x,part.width,frame.width,mirrored),y+part.forward,
                part.width,part.length,altitude+part.bottom,altitude+part.top,part.color,monochrome);
        }
        for(const auto& triangle:frame.triangles)
        {
            bool before=true,after=true;
            for(const auto& vertex:triangle.vertices)
            {
                before &= y+vertex.forward < -ValleyHistoryLines;
                after &= y+vertex.forward >= FieldHeight;
            }
            if(before || after) continue;
            Paint(triangle.color,monochrome);
            glBegin(GL_TRIANGLES);
            for(const auto& vertex:triangle.vertices)
                glVertex3f(x+(mirrored?frame.width-vertex.x:vertex.x),altitude+vertex.altitude,-(y+vertex.forward));
            glEnd();
        }
    }

    void DrawModelShadow(const AssetFrame& frame,float x,float y,float altitude,bool mirrored,bool monochrome)
    {
        for(const auto& run:frame.pixels)
        {
            if(y+run.y < -ValleyHistoryLines || y+run.y>=FieldHeight) continue;
            Box(x+MirrorX(run.x,run.width,frame.width,mirrored),y+run.y,run.width,1,
                altitude,altitude+0.02f,{0.035f,0.10f,0.15f},monochrome);
        }
    }
}
